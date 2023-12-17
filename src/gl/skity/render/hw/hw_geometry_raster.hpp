#ifndef SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP

#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "gl/skity/geometry/rect.hpp"
#include "gl/skity/graphic/paint.hpp"
#include "gl/skity/utils/lazy.hpp"

namespace skity {

    class HWMesh;

    class HWGeometryRaster
    {
    public:
        HWGeometryRaster(HWMesh* mesh, const Paint& paint, bool use_gs);
        virtual ~HWGeometryRaster() = default;

        void RasterLine(const glm::vec2& p0, const glm::vec2& p1);
        void RasterRect(const Rect& rect);
        void FillCircle(float cx, float cy, float radius);
        void FillTextRect(const glm::vec4& bounds, const glm::vec2& uv_lt, const glm::vec2& uv_rb);

        void ResetRaster();
        void FlushRaster();

        uint32_t StencilFrontStart() const
        {
            return stencil_front_start_;
        }

        uint32_t StencilFrontCount() const
        {
            return stencil_front_count_;
        }

        uint32_t StencilBackStart() const
        {
            return stencil_back_start_;
        }

        uint32_t StencilBackCount() const
        {
            return stencil_back_count_;
        }

        uint32_t ColorStart() const
        {
            return color_start_;
        }

        uint32_t ColorCount() const
        {
            return color_count_;
        }

        Rect RasterBounds() const;

        bool UseGeometryShader() const
        {
            return use_gs_;
        }

    protected:
        enum BufferType {
            kStencilFront,
            kStencilBack,
            kColor,
        };

        void SetBufferType(BufferType type)
        {
            buffer_type_ = type;
        }

        float StrokeWidth() const;

        float StrokeMiter() const
        {
            return paint_.getStrokeMiter();
        }

        Paint::Cap LineCap() const
        {
            return paint_.getStrokeCap();
        }

        Paint::Join LineJoin() const
        {
            return paint_.getStrokeJoin();
        }

        void ChangeLineJoin(Paint::Join join)
        {
            paint_.setStrokeJoin(join);
        }

        void HandleLineCap(const glm::vec2& center, const glm::vec2& p0, const glm::vec2& p1,
                           const glm::vec2& out_dir, float stroke_radius);

        std::array<glm::vec2, 4> ExpandLine(const glm::vec2& p0, const glm::vec2& p1,
                                            float stroke_radius);

        uint32_t AppendLineVertex(const glm::vec2& p);
        uint32_t AppendCircleVertex(const glm::vec2& p, const glm::vec2& center);
        uint32_t AppendVertex(float x, float y, float mix, float u, float v);

        void AppendRect(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

        void AppendFrontTriangle(uint32_t a, uint32_t b, uint32_t c);
        void AppendBackTriangle(uint32_t a, uint32_t b, uint32_t c);

        void FillRect(const Rect& rect);
        void StrokeRect(const Rect& rect);

        // used for convexity polygon
        void SwitchStencilToColor();

        void HandleGSRoundCap(const glm::vec2& center, const glm::vec2& p0, const glm::vec2& p1,
                              const glm::vec2& out_dir, float stroke_radius);

        void ExpandBounds(const glm::vec2& p);

        void GenerateCircleMesh(const Vec2& center, const Vec2& p1, const Vec2& p2);

    private:
        std::vector<uint32_t>& CurrentIndexBuffer();

    private:
        HWMesh* mesh_;
        Paint paint_;
        bool use_gs_;
        BufferType buffer_type_ = kColor;

        uint32_t stencil_front_start_ = {};
        uint32_t stencil_front_count_ = {};
        uint32_t stencil_back_start_ = {};
        uint32_t stencil_back_count_ = {};
        uint32_t color_start_ = {};
        uint32_t color_count_ = {};

        std::vector<uint32_t> stencil_front_buffer_ = {};
        std::vector<uint32_t> stencil_back_buffer_ = {};
        std::vector<uint32_t> color_buffer_ = {};

        Lazy<glm::vec4> bounds_ = {};
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_GEOMETRY_RASTER_HPP
