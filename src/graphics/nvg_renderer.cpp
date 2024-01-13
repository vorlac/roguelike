#include <glad/gl.h>

#include "core/ui/theme.hpp"
#include "graphics/nvg_renderer.hpp"
#include "graphics/vg/nanovg.hpp"
#include "graphics/vg/nanovg_gl.hpp"
#include "graphics/vg/nanovg_gl_utils.hpp"

namespace rl {
    using namespace vg;

    struct Property
    {
        enum Flags {
            // Flag indicating if geometry based anti-aliasing is used.
            // May not be needed when using MSAA.
            AntiAlias = NVGcreateFlags::NVG_ANTIALIAS,
            // Flag indicating if strokes should be drawn using stencil buffer. The rendering
            // will be a little slower, but path overlaps (i.e. self-intersecting or sharp
            // turns) will be drawn just once.
            StencilStrokes = NVGcreateFlags::NVG_STENCIL_STROKES,
            // Flag indicating that additional debug checks are done.
            Debug = NVGcreateFlags::NVG_DEBUG,
        };
    };

    static NVGcontext* create_nanovg_context(bool& stencil_buf, bool& depth_buf, bool& float_buf)
    {
        u8 float_mode{ 0 };
        i32 depth_bits{ 0 };
        i32 stencil_bits{ 0 };

        // TODO: look into why this returns an error code?..
        // glGetBooleanv(GL_RGBA_FLOAT_MODE_ARB, &float_mode);
        glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil_bits);

        stencil_buf = stencil_bits > 0;
        depth_buf = depth_bits > 0;
        float_buf = float_mode != 0;

        i32 nvg_flags{ Property::AntiAlias };
        if (stencil_buf)
            nvg_flags |= Property::StencilStrokes;

#ifndef NDEBUG
        nvg_flags |= Property::Debug;
#endif
        NVGcontext* nvg_context{ nvgCreateGL3(nvg_flags) };
        runtime_assert(nvg_context != nullptr, "Failed to create NVG context");
        return nvg_context;
    };

    VectorizedRenderer::VectorizedRenderer()
        : m_nvg_context{ rl::create_nanovg_context(m_stencil_buffer, m_depth_buffer,
                                                   m_float_buffer) }
    {
    }

    NVGcontext* VectorizedRenderer::nvg_context() const
    {
        return m_nvg_context;
    }

    void VectorizedRenderer::flush(ds::dims<i32> viewport, f32 pixel_ratio)
    {
        // Flush all queued up NanoVG rendering commands
        NVGparams* params{ nvgInternalParams(m_nvg_context) };
        params->renderFlush(params->userPtr);
        params->renderViewport(params->userPtr, static_cast<f32>(viewport.width),
                               static_cast<f32>(viewport.height), pixel_ratio);
    }

    void VectorizedRenderer::push_render_state()
    {
        nvgSave(m_nvg_context);
    }

    void VectorizedRenderer::pop_render_state()
    {
        nvgRestore(m_nvg_context);
    }

    void VectorizedRenderer::draw_rect_outline(ds::rect<i32> rect, f32 stroke_width,
                                               ds::color<f32> color, ui::Outline type)
    {
        nvgStrokeWidth(m_nvg_context, stroke_width);
        nvgBeginPath(m_nvg_context);

        switch (type)
        {
            case ui::Outline::Inner:
                nvgRect(m_nvg_context, rect.pt.x + (stroke_width / 2.0f),
                        rect.pt.y + (stroke_width / 2.0f), rect.size.width - stroke_width,
                        rect.size.height - stroke_width);
                break;
            case ui::Outline::Outer:
                nvgRect(m_nvg_context, rect.pt.x - (stroke_width / 2.0f),
                        rect.pt.y - (stroke_width / 2.0f), rect.size.width + stroke_width,
                        rect.size.height + stroke_width);
                break;
        }

        nvgStrokeColor(m_nvg_context, std::forward<NVGcolor>(color));
        nvgStroke(m_nvg_context);
    }
}
