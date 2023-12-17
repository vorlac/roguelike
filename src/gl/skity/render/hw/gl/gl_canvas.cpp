#include "gl/skity/render/hw/gl/gl_canvas.hpp"
#include "gl/skity/render/hw/gl/gl_font_texture.hpp"
#include "gl/skity/render/hw/gl/gl_interface.hpp"
#include "gl/skity/render/hw/gl/gl_render_target.hpp"
#include "gl/skity/render/hw/gl/gl_texture.hpp"

#ifdef SKITY_ANDROID
  #include <cstdio>
#endif

namespace skity {

    GLCanvas::GLCanvas(Matrix mvp, uint32_t width, uint32_t height, float density)
        : HWCanvas(mvp, width, height, density)
    {
    }

    void GLCanvas::OnInit(GPUContext* ctx)
    {
        ctx_ = ctx;
    }

    bool GLCanvas::SupportGeometryShader()
    {
        // FIXME
        // enable GS Shader by default in OpenGL
        return true;
    }

    std::unique_ptr<HWRenderer> GLCanvas::CreateRenderer()
    {
        auto renderer = std::make_unique<GLRenderer>(ctx_, SupportGeometryShader());
        renderer->Init();

        gl_renderer_ = renderer.get();
        return renderer;
    }

    std::unique_ptr<HWTexture> GLCanvas::GenerateTexture()
    {
        return std::make_unique<GLTexture>();
    }

    std::unique_ptr<HWFontTexture> GLCanvas::GenerateFontTexture(Typeface* typeface)
    {
        return std::make_unique<GLFontTexture>(typeface);
    }

    std::unique_ptr<HWRenderTarget> GLCanvas::GenerateBackendRenderTarget(uint32_t width,
                                                                          uint32_t height)
    {
        auto fbo = std::make_unique<GLRenderTarget>(width, height);

        // always enable multisample fbo
        fbo->SetEnableMultiSample(true);

        fbo->Init();

        return fbo;
    }

}  // namespace skity
