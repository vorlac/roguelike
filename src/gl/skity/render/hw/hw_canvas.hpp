#ifndef SKITY_SRC_RENDER_HW_HW_CANVAS_HPP
#define SKITY_SRC_RENDER_HW_HW_CANVAS_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gl/skity/geometry/point.hpp"
#include "gl/skity/render/canvas.hpp"
#include "gl/skity/render/hw/hw_canvas_state.hpp"
#include "gl/skity/render/hw/hw_draw.hpp"
#include "gl/skity/render/hw/hw_font_texture.hpp"
#include "gl/skity/render/hw/hw_render_target.hpp"
#include "gl/skity/render/hw/hw_texture.hpp"
#include "gl/skity/utils/lazy.hpp"

namespace skity {

    class Pixmap;
    class Typeface;

    class HWMesh;
    class HWRenderer;
    class HWPathRaster;

    /**
     * @class HWCanvas
     *  Base class for all hardware canvas implementation, use MSAA for anti-alias
     */
    class HWCanvas : public Canvas
    {
        using DrawList = std::vector<std::unique_ptr<HWDraw>>;

    public:
        HWCanvas(Matrix mvp, uint32_t width, uint32_t height, float density);
        ~HWCanvas() override;

        void Init(GPUContext* ctx);

    protected:
        virtual void OnInit(GPUContext* ctx) = 0;

        virtual bool SupportGeometryShader() = 0;
        virtual std::unique_ptr<HWRenderer> CreateRenderer() = 0;
        virtual std::unique_ptr<HWTexture> GenerateTexture() = 0;
        virtual std::unique_ptr<HWFontTexture> GenerateFontTexture(Typeface* typeface) = 0;
        virtual std::unique_ptr<HWRenderTarget> GenerateBackendRenderTarget(uint32_t width,
                                                                            uint32_t height) = 0;

        void onDrawLine(float x0, float y0, float x1, float y1, const Paint& paint) override;

        void onDrawRect(const Rect& rect, const Paint& paint) override;

        void onClipPath(const Path& path, ClipOp op) override;

        void onDrawPath(const Path& path, const Paint& paint) override;

        void onDrawBlob(const TextBlob* blob, float x, float y, const Paint& paint) override;

        void onSave() override;

        void onRestore() override;

        void onRestoreToCount(int saveCount) override;

        void onTranslate(float dx, float dy) override;

        void onScale(float sx, float sy) override;

        void onRotate(float degree) override;

        void onRotate(float degree, float px, float py) override;

        void onConcat(const Matrix& matrix) override;

        void onFlush() override;

        uint32_t onGetWidth() const override;

        uint32_t onGetHeight() const override;

        void onUpdateViewport(uint32_t width, uint32_t height) override;

        HWMesh* GetMesh();

        glm::mat4 GetCurrentMVP() const
        {
            return mvp_;
        }

        void SetCurrentMVP(const glm::mat4& mvp)
        {
            mvp_ = mvp;
        }

    private:
        std::unique_ptr<HWDraw> GenerateOp();

        std::unique_ptr<HWDraw> GenerateColorOp(const Paint& paint, bool stroke = false,
                                                const Rect& = {});

        HWRenderer* GetPipeline()
        {
            return renderer_.get();
        }

        HWTexture* QueryTexture(Pixmap* pixmap);
        HWFontTexture* QueryFontTexture(Typeface* typeface);
        HWRenderTarget* QueryRenderTarget(const Rect& bounds);

        float FillTextRun(float x, float y, const TextRun& run, const Paint& paint);
        float FillTextRunWithPath(float x, float y, const TextRun& run, const Paint& paint);
        float StrokeTextRun(float x, float y, const TextRun& run, const Paint& paint);

        void ClearClipMask();
        void ForwardFillClipMask();

        DrawList& CurrentDrawList();

        void PushDrawList();

        DrawList PopDrawList();

        void ClearDrawList();

        void EnqueueDrawOp(std::unique_ptr<HWDraw> draw);

        void EnqueueDrawOp(std::unique_ptr<HWDraw> draw, const Rect& bounds,
                           const std::shared_ptr<MaskFilter>& mask_filter);

        void HandleMaskFilter(DrawList draw_list, const Rect& bounds,
                              const std::shared_ptr<MaskFilter>& mask_filter);

    private:
        Matrix mvp_;
        uint32_t width_;
        uint32_t height_;
        int32_t full_rect_start_ = -1;
        int32_t full_rect_count_ = -1;
        float density_ = 2.f;
        HWCanvasState state_;
        std::unique_ptr<HWMesh> mesh_;
        Lazy<float> global_alpha_ = {};
        std::unique_ptr<HWRenderer> renderer_ = {};
        std::vector<DrawList> draw_list_stack_ = {};
        std::map<Pixmap*, std::unique_ptr<HWTexture>> image_texture_store_ = {};
        std::map<Typeface*, std::unique_ptr<HWFontTexture>> font_texture_store_ = {};
        HWRenderTargetCache render_target_cache_ = {};
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_CANVAS_HPP
