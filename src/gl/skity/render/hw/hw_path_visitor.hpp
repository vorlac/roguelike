#ifndef SKITY_SRC_RENDER_HW_HW_PATH_VISITOR_HPP
#define SKITY_SRC_RENDER_HW_HW_PATH_VISITOR_HPP

#include "gl/skity/graphic/paint.hpp"
#include "gl/skity/graphic/path.hpp"
#include "gl/skity/render/hw/hw_geometry_raster.hpp"

namespace skity {

    struct HWDrawRange;
    class HWMesh;

    class HWPathVisitor : public HWGeometryRaster
    {
    public:
        HWPathVisitor(HWMesh* mesh, const Paint& paint, bool use_gs)
            : HWGeometryRaster(mesh, paint, use_gs)
        {
        }

        virtual ~HWPathVisitor() = default;

        void VisitPath(const Path& path, bool force_close);

    private:
        void HandleMoveTo(const glm::vec2& p);
        void HandleLineTo(const glm::vec2& p1, const glm::vec2& p2);
        void HandleQuadTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3);
        void HandleConicTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                           float weight);
        void HandleCubicTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3,
                           const glm::vec2& p4);
        void HandleClose();

    protected:
        glm::vec2 FirstPoint() const
        {
            return first_pt_;
        }

        glm::vec2 PrevDir() const
        {
            return prev_dir_;
        }

        virtual void OnBeginPath() = 0;

        virtual void OnEndPath() = 0;

        virtual void OnMoveTo(const glm::vec2& p) = 0;

        virtual void OnLineTo(const glm::vec2& p1, const glm::vec2& p2) = 0;

        virtual void OnQuadTo(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) = 0;

    private:
        glm::vec2 first_pt_ = {};
        glm::vec2 prev_dir_ = {};
        glm::vec2 prev_pt_ = {};
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_PATH_VISITOR_HPP
