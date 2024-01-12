#include <glad/gl.h>

#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>

#include "core/ui/theme.hpp"
#include "render/vectorized_renderer.hpp"

namespace rl {
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

    static NVGcontext* create_nanovg_context()
    {
        i32 nvg_flags{ Property::AntiAlias };
        nvg_flags |= Property::Debug;
        bool stencil_buffer = true;
        if (stencil_buffer)
            nvg_flags |= Property::StencilStrokes;

        NVGcontext* nvg_context{ nvgCreateGL3(nvg_flags) };
        runtime_assert(nvg_context != nullptr, "Failed to create NVG context");
        return nvg_context;
    };

    VectorizedRenderer::VectorizedRenderer()
        : m_nvg_context{ create_nanovg_context() }
    {
    }

    VectorizedRenderer::VectorizedRenderer(NVGcontext* context)
        : m_nvg_context{ context }
    {
        runtime_assert(context != nullptr, "VectorizedRenderer: invalid NV Context");
    }

    NVGcontext* VectorizedRenderer::nvg_context() const
    {
        return m_nvg_context;
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
