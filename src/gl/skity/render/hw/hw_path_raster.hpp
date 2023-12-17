#ifndef SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP
#define SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP

#include <vector>

#include "gl/skity/render/hw/hw_path_visitor.hpp"

namespace skity {

    class HWMesh;

    enum class Orientation;

    class HWPathRaster : public HWPathVisitor
    {
    public:
        HWPathRaster(HWMesh* mesh, const Paint& paint, bool use_gs)
            : HWPathVisitor(mesh, paint, use_gs)
        {
        }

        ~HWPathRaster() override = default;

        void FillPath(const Path& path);
        void StrokePath(const Path& path);

    protected:
        void OnBeginPath() override;
        void OnEndPath() override;
        void OnMoveTo(const glm::vec2& p) override;

        void OnLineTo(const glm::vec2& p1, const glm::vec2& p2) override;

        void OnQuadTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) override;

    private:
        void StrokeLineTo(const glm::vec2& p1, const glm::vec2& p2);
        void StrokeQuadTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3);
        void GSStrokeQuadTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3);
        void FillLineTo(const glm::vec2& p1, const glm::vec2& p2);
        void FillQuadTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3);
        void HandleLineJoin(const glm::vec2& p1, const glm::vec2& p2, float stroke_radius);

        void HandleMiterJoinInternal(const Vec2& center, const Vec2& p1, const Vec2& d1,
                                     const Vec2& p2, const Vec2& d2);

        void HandleBevelJoinInternal(const Vec2& center, const Vec2& p1, const Vec2& p2,
                                     const Vec2& curr_dir);

        void HandleRoundJoinInternal(const Vec2& center, const Vec2& p1, const Vec2& d1,
                                     const Vec2& p2, const Vec2& d2);

        void HandleRoundJoinWithGS(const Vec2& center, const Vec2& p1, const Vec2& d1,
                                   const Vec2& p2, const Vec2& d2);

        void GSFillQuad(Orientation orientation, const glm::vec2& p1, const glm::vec2& p2,
                        const glm::vec2& p3);

    private:
        bool stroke_ = false;
        glm::vec2 first_pt_ = {};
        int32_t first_pt_index_ = -1;
        glm::vec2 first_pt_dir_ = {};
        glm::vec2 prev_pt_ = {};
        glm::vec2 curr_pt_ = {};
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_PATH_RASTER_HPP
