#ifndef SKITY_SRC_RENDER_HW_HW_DRAW_HPP
#define SKITY_SRC_RENDER_HW_HW_DRAW_HPP

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "gl/skity/effect/mask_filter.hpp"
#include "gl/skity/geometry/rect.hpp"
#include "gl/skity/utils/lazy.hpp"

namespace skity {

    struct HWDrawRange
    {
        uint32_t start = 0;
        uint32_t count = 0;
    };

    class HWRenderer;
    class HWTexture;
    class HWRenderTarget;

    class HWDraw
    {
    public:
        HWDraw(HWRenderer* renderer, bool has_clip, bool clip_stencil = false)
            : renderer_(renderer)
            , has_clip_(has_clip)
            , clip_stencil_(clip_stencil)
        {
        }

        virtual ~HWDraw() = default;

        virtual void Draw();

        void SetPipelineColorMode(uint32_t mode);

        void SetStencilRange(const HWDrawRange& front_range, const HWDrawRange& back_range);

        void SetColorRange(const HWDrawRange& color_range);

        void SetStrokeWidth(float width);

        void SetUniformColor(const glm::vec4& color);

        void SetTransformMatrix(const glm::mat4& matrix);

        void SetGradientBounds(const glm::vec2& p0, const glm::vec2& p1);

        void SetGradientColors(const std::vector<glm::vec4>& colors);

        void SetGradientPositions(const std::vector<float>& pos);

        void SetClearStencilClip(bool clear);

        void SetTexture(HWTexture* texture);

        void SetFontTexture(HWTexture* font_texture);

        void SetGlobalAlpha(float alpha);

        void SetHasClip(bool has_clip)
        {
            has_clip_ = has_clip;
        }

        void SetEvenOddFill(bool is_even_odd)
        {
            even_odd_fill_ = is_even_odd;
        }

    protected:
        HWRenderer* GetPipeline()
        {
            return renderer_;
        }

        bool HasClip()
        {
            return has_clip_;
        }

        const Lazy<glm::mat4>& TransformMatrix() const
        {
            return transform_matrix_;
        }

    private:
        void DoStencilIfNeed();
        void DoColorFill();
        void DoStencilBufferMove();

        void DoStencilBufferMoveInternal();
        void DoStencilBufferClearIfNeed();

        void HandleStencilDiscard();
        void HandleNormalStencilDiscard();
        void HandleEvenOddStencilDiscard();
        void BindTexture();

    private:
        HWRenderer* renderer_;
        bool has_clip_;
        bool clip_stencil_;
        bool clear_stencil_clip_ = false;
        uint32_t pipeline_type_ = 0;
        uint32_t pipeline_mode_ = 0;
        HWDrawRange stencil_front_range_ = {};
        HWDrawRange stencil_back_range_ = {};
        HWDrawRange color_range_ = {};
        bool even_odd_fill_ = false;
        Lazy<float> stroke_width_ = {};
        Lazy<glm::vec4> uniform_color_ = {};
        Lazy<glm::mat4> transform_matrix_ = {};
        Lazy<glm::vec4> gradient_bounds_ = {};
        Lazy<float> global_alpha_ = {};
        std::vector<glm::vec4> gradient_colors_ = {};
        std::vector<float> gradient_stops_ = {};
        HWTexture* texture_ = {};
        HWTexture* font_texture_ = {};
    };

    class PostProcessDraw : public HWDraw
    {
    public:
        PostProcessDraw(HWRenderTarget* render_target,
                        std::vector<std::unique_ptr<HWDraw>> draw_list, const Rect& bounds,
                        HWRenderer* renderer, bool has_clip, bool clip_stencil = false);

        PostProcessDraw(HWRenderTarget* render_target, std::unique_ptr<HWDraw> op,
                        const Rect& bounds, HWRenderer* renderer, bool has_clip,
                        bool clip_stencil = false);

        ~PostProcessDraw() override;

        void SetBlurStyle(BlurStyle style)
        {
            blur_style_ = style;
        }

        void SetBlurRadius(float sigma)
        {
            blur_radius_ = sigma;
        }

        void Draw() override;

    private:
        void DrawToRenderTarget();

        void DoFilter();

        void DrawToCanvas();

        void SaveTransform();
        void RestoreTransform();

    private:
        HWRenderTarget* render_target_ = {};
        std::vector<std::unique_ptr<HWDraw>> draw_list_ = {};
        Rect bounds_ = {};
        BlurStyle blur_style_ = BlurStyle::kNormal;
        float blur_radius_ = 0.f;
        glm::mat4 saved_mvp_ = {};
        glm::mat4 saved_transform_ = {};
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_DRAW_HPP
