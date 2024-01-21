#include <glad/gl.h>

#include <memory>
#include <type_traits>

#include <parallel_hashmap/phmap.h>

#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "graphics/nvg_renderer.hpp"
#include "graphics/vg/nanovg.hpp"
#include "graphics/vg/nanovg_gl.hpp"
#include "graphics/vg/nanovg_gl_utils.hpp"

namespace rl {

    struct Property
    {
        enum Flags {
            // Flag indicating if geometry based anti-aliasing is used.
            // May not be needed when using MSAA.
            AntiAlias = nvg::NVGcreateFlags::NVG_ANTIALIAS,
            // Flag indicating if strokes should be drawn using stencil buffer. The rendering
            // will be a little slower, but path overlaps (i.e. self-intersecting or sharp
            // turns) will be drawn just once.
            StencilStrokes = nvg::NVGcreateFlags::NVG_STENCIL_STROKES,
            // Flag indicating that additional debug checks are done.
            Debug = nvg::NVGcreateFlags::NVG_DEBUG,
        };
    };

    static nvg::NVGcontext* create_nanovg_context(bool& stencil_buf, bool& depth_buf,
                                                  bool& float_buf)
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
        nvg::NVGcontext* nvg_context{ nvg::nvgCreateGL3(nvg_flags) };
        runtime_assert(nvg_context != nullptr, "Failed to create NVG context");
        return nvg_context;
    };

    NVGRenderer::NVGRenderer()
        : m_nvg_context{ rl::create_nanovg_context(m_stencil_buffer, m_depth_buffer,
                                                   m_float_buffer) }
    {
        this->load_fonts({
            //{
            //    ui::font::name::sans,
            //    std::basic_string_view<u8>{
            //        roboto_regular_ttf,
            //        roboto_regular_ttf_size,
            //    },
            //},
            //{
            //    ui::font::name::sans_bold,
            //    std::basic_string_view<u8>{
            //        roboto_bold_ttf,
            //        roboto_bold_ttf_size,
            //    },
            //},
            //{
            //    ui::font::name::mono,
            //    std::basic_string_view<u8>{
            //        fontawesome_solid_ttf,
            //        fontawesome_solid_ttf_size,
            //    },
            //},
        });
    }

    nvg::NVGcontext* NVGRenderer::context() const
    {
        return m_nvg_context;
    }

    void NVGRenderer::begin_frame(ds::dims<f32> render_size, f32 pixel_ratio)
    {
        nvg::BeginFrame(m_nvg_context, render_size.width, render_size.height, pixel_ratio);
    }

    void NVGRenderer::end_frame()
    {
        nvg::EndFrame(m_nvg_context);
    }

    void NVGRenderer::render_frame(ds::dims<f32> render_size, f32 pixel_ratio, auto&& render_func)
    {
        this->begin_frame(render_size, pixel_ratio);
        render_func();
        this->end_frame();
    }

    void NVGRenderer::flush(ds::dims<i32> viewport, f32 pixel_ratio)
    {
        // Flush all queued up NanoVG rendering commands
        nvg::NVGparams* params{ nvg::InternalParams(m_nvg_context) };
        params->renderFlush(params->userPtr);
        params->renderViewport(params->userPtr, static_cast<f32>(viewport.width),
                               static_cast<f32>(viewport.height), pixel_ratio);
    }

    void NVGRenderer::push_render_state()
    {
        nvg::Save(m_nvg_context);
    }

    void NVGRenderer::pop_render_state()
    {
        nvg::Restore(m_nvg_context);
    }

    void NVGRenderer::scoped_state_draw(auto&& draw_func)
    {
        this->push_render_state();
        draw_func();
        this->pop_render_state();
    }

    ui::font::Handle NVGRenderer::load_font(std::string_view font_name,
                                            std::basic_string_view<u8>&& font_ttf)
    {
        return nvg::CreateFontMem(m_nvg_context, font_name.data(), const_cast<u8*>(font_ttf.data()),
                                  static_cast<i32>(font_ttf.size()), 0);
    }

    void NVGRenderer::load_fonts(std::vector<FontInfo>&& fonts)
    {
        for (auto&& font_info : fonts)
        {
            auto&& [font_name, font_ttf] = font_info;
            auto handle{ this->load_font(font_name, std::forward<decltype(font_ttf)>(font_ttf)) };
            m_font_map[font_name] = handle;
        }
    }

    void NVGRenderer::set_text_properties(std::string_view& font_name, f32 font_size,
                                          ui::Text::Alignment alignment) const
    {
        nvg::FontFace(m_nvg_context, font_name.data());
        nvg::FontSize(m_nvg_context, font_size);
        nvg::TextAlign(m_nvg_context, alignment);
    }

    ds::rect<f32> NVGRenderer::get_text_box_rect(
        const std::string& text, const ds::point<f32>& pos, std::string_view& font_name,
        f32 font_size, f32 fold_width, ui::Text::Alignment alignment) const
    {
        this->set_text_properties(font_name, font_size, alignment);

        std::array<f32, 4> bounds{ 0.0f };
        nvg::TextBoxBounds(m_nvg_context, pos.x, pos.y, fold_width, text.data(), nullptr,
                           &bounds.front());

        return ds::rect<f32>{
            pos,
            ds::dims{
                fold_width,
                bounds[3] - bounds[1],
            },
        };
    }

    ds::dims<f32> NVGRenderer::get_text_size(const std::string& text, std::string_view& font_name,
                                             f32 font_size, ui::Text::Alignment alignment) const
    {
        this->set_text_properties(font_name, font_size, alignment);

        f32 width{ nvg::TextBounds(m_nvg_context, 0.0f, 0.0f, text.data(), nullptr, nullptr) };

        constexpr static f32 WIDTH_BUFFER{ 2.0f };
        return ds::dims<f32>{
            width + WIDTH_BUFFER,
            font_size,
        };
    }

    void NVGRenderer::draw_rect_outline(ds::rect<f32> rect, f32 stroke_width, ds::color<f32> color,
                                        ui::Outline type)
    {
        nvg::StrokeWidth(m_nvg_context, stroke_width);
        nvg::BeginPath(m_nvg_context);

        switch (type)
        {
            case ui::Outline::Inner:
                nvg::Rect(m_nvg_context, rect.pt.x + (stroke_width / 2.0f),
                          rect.pt.y + (stroke_width / 2.0f), rect.size.width - stroke_width,
                          rect.size.height - stroke_width);
                break;
            case ui::Outline::Outer:
                nvg::Rect(m_nvg_context, rect.pt.x - (stroke_width / 2.0f),
                          rect.pt.y - (stroke_width / 2.0f), rect.size.width + stroke_width,
                          rect.size.height + stroke_width);
                break;
        }

        nvg::StrokeColor(m_nvg_context, color);
        nvg::Stroke(m_nvg_context);
    }
}
