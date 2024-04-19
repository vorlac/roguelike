#include <glad/gl.h>

#include <bit>
#include <memory>
#include <type_traits>

// #include "ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/line.hpp"
#include "graphics/nvg_renderer.hpp"
#include "graphics/text.hpp"
#include "graphics/vg/nanovg.hpp"
#include "graphics/vg/nanovg_gl.hpp"

namespace rl {
    namespace {
        nvg::Context* create_nvg_context(bool& stencil_buf, bool& depth_buf, bool& float_buf)
        {
            constexpr u8 float_mode{ 0 };
            i32 depth_bits{ 0 };
            i32 stencil_bits{ 0 };

            // TODO: look into why this returns an error code?..
            // glGetBooleanv(GL_RGBA_FLOAT_MODE_ARB, &float_mode);

            glGetFramebufferAttachmentParameteriv(
                GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);

            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_STENCIL,
                                                  GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
                                                  &stencil_bits);
            stencil_buf = stencil_bits > 0;
            depth_buf = depth_bits > 0;
            float_buf = float_mode != 0;

            constexpr nvg::gl::CreateFlags nvg_flags{ nvg::gl::CreateFlags::AntiAlias |
                                                      nvg::gl::CreateFlags::StencilStrokes };
            // if (stencil_buf)
            //     nvg_flags |= nvg::gl::CreateFlags::StencilStrokes;
            // nvg_flags |= nvg::gl::CreateFlags::Debug;

            nvg::Context* nvg_context{ nvg::gl::create_gl_context(nvg_flags) };
            runtime_assert(nvg_context != nullptr, "Failed to create NVG context");
            return nvg_context;
        };
    }

    NVGRenderer::NVGRenderer()
        : m_nvg_context{ create_nvg_context(m_stencil_buffer, m_depth_buffer, m_float_buffer) }
    {
        this->load_fonts({
            text::font::Data{
                text::font::style::Sans,
                std::basic_string_view{
                    roboto_regular_ttf,
                    roboto_regular_ttf_size,
                },
            },
            text::font::Data{
                text::font::style::SansBold,
                std::basic_string_view{
                    roboto_bold_ttf,
                    roboto_bold_ttf_size,
                },
            },
            text::font::Data{
                text::font::style::Icons,
                std::basic_string_view{
                    fontawesome_solid_ttf,
                    fontawesome_solid_ttf_size,
                },
            },
            text::font::Data{
                text::font::style::Mono,
                std::basic_string_view{
                    fira_code_bold_ttf,
                    fira_code_bold_ttf_size,
                },
            },
        });
    }

    nvg::Context* NVGRenderer::context() const
    {
        return m_nvg_context.get();
    }

    void NVGRenderer::begin_frame(const ds::dims<f32>& render_size, const f32 pixel_ratio) const
    {
        nvg::begin_frame(m_nvg_context.get(), render_size.width, render_size.height, pixel_ratio);
    }

    void NVGRenderer::end_frame() const
    {
        nvg::end_frame(m_nvg_context.get());
    }

    void NVGRenderer::save_state() const
    {
        nvg::save(m_nvg_context.get());
    }

    void NVGRenderer::begin_path() const
    {
        nvg::begin_path(m_nvg_context.get());
    }

    void NVGRenderer::end_path() const
    {
        nvg::begin_path(m_nvg_context.get());
    }

    void NVGRenderer::restore_state() const
    {
        nvg::restore(m_nvg_context.get());
    }

    void NVGRenderer::reset_scissor() const
    {
        nvg::reset_scissor(m_nvg_context.get());
    }

    nvg::PaintStyle NVGRenderer::create_rect_gradient_paint_style(ds::rect<f32>&& rect, const f32 corner_radius,
                                                                  const f32 outer_blur, const ds::color<f32>& inner_color,
                                                                  const ds::color<f32>& outer_gradient_color) const
    {
        // Creates and returns a box gradient.
        // Box gradient is a feathered rounded rectangle, it is useful for rendering drop shadows or
        // highlights for boxes. Parameters (x,y) define the top-left corner of the rectangle, (w,h)
        // define the size of the rectangle, r defines the corner radius, and f feather. Feather
        // defines how blurry the border of the rectangle is. Parameter icol specifies the inner
        // color and ocol the outer color of the gradient. The gradient is transformed by the
        // current transform when it is passed to FillPaint() or StrokePaint().
        return nvg::box_gradient(m_nvg_context.get(), std::forward<ds::rect<f32>>(rect),
                                 corner_radius, outer_blur, inner_color, outer_gradient_color);
    }

    nvg::PaintStyle NVGRenderer::create_linear_gradient_paint_style(ds::line<f32>&& line,
                                                                    const ds::color<f32>& inner_color,
                                                                    const ds::color<f32>& outer_gradient_color) const
    {
        // Creates and returns a box gradient.
        // Box gradient is a feathered rounded rectangle, it is useful for rendering drop shadows or
        // highlights for boxes. Parameters (x,y) define the top-left corner of the rectangle, (w,h)
        // define the size of the rectangle, r defines the corner radius, and f feather. Feather
        // defines how blurry the border of the rectangle is. Parameter icol specifies the inner
        // color and ocol the outer color of the gradient. The gradient is transformed by the
        // current transform when it is passed to FillPaint() or StrokePaint().
        return nvg::linear_gradient(m_nvg_context.get(), line.start.x, line.start.y, line.end.x,
                                    line.end.y, inner_color, outer_gradient_color);
    }

    text::font::handle NVGRenderer::load_font(const std::string_view& font_name,
                                              const std::basic_string_view<u8>& font_ttf) const
    {
        // Creates font by loading it from the specified memory chunk.
        // Returns handle to the font.
        text::font::handle fh{ nvg::create_font_mem(m_nvg_context.get(), font_name, font_ttf) };
        runtime_assert(fh != text::font::InvalidHandle, "failed to load font: {}", font_name);
        return fh;
    }

    void NVGRenderer::flush(const ds::dims<f32>& viewport, const f32 pixel_ratio) const
    {
        // Flush all queued up NanoVG rendering commands
        nvg::Params* params{ nvg::internal_params(m_nvg_context.get()) };
        params->render_flush(params->user_ptr);
        params->render_viewport(params->user_ptr, viewport.width, viewport.height, pixel_ratio);
    }

    void NVGRenderer::load_fonts(const std::vector<text::font::Data>& fonts)
    {
        for (auto&& [font_name, font_ttf] : fonts) {
            text::font::handle fh{ this->load_font(font_name, font_ttf) };
            m_font_map[font_name] = fh;
        }
    }

    void NVGRenderer::set_fill_paint_style(nvg::PaintStyle&& paint_style) const
    {
        nvg::fill_paint(m_nvg_context.get(), std::move(paint_style));
    }

    void NVGRenderer::fill_current_path(nvg::PaintStyle&& paint_style) const
    {
        nvg::fill_paint(m_nvg_context.get(), std::move(paint_style));
        nvg::fill(m_nvg_context.get());
    }

    void NVGRenderer::set_text_properties_(const std::string_view& font_name, const f32 font_size,
                                           const Align alignment) const
    {
        nvg::set_font_face(m_nvg_context.get(), font_name);
        nvg::set_font_size(m_nvg_context.get(), font_size);
        nvg::set_text_align(m_nvg_context.get(), alignment);
    }

    ds::dims<f32> NVGRenderer::get_text_size(const std::string&) const
    {
        assert_cond(false);
        return ds::dims<f32>::zero();
    }

    ds::rect<f32> NVGRenderer::get_text_box_rect(
        const std::string& text, const ds::point<f32>& pos, const std::string_view& font_name,
        const f32 font_size, const f32 fold_width, const Align alignment) const
    {
        this->set_text_properties_(font_name, font_size, alignment);
        // Measures the specified multi-text string. Parameter bounds should be a pointer to
        // float[4], if the bounding box of the text should be returned. The bounds value are
        // [xmin,ymin, xmax,ymax] Measured values are returned in local coordinate space.
        ds::rect bounds{ nvg::text_box_bounds(m_nvg_context.get(), pos, fold_width, text) };

        return ds::rect<f32>{
            pos,
            ds::dims<f32>{
                fold_width,
                bounds.size.height,
            },
        };
    }

    ds::dims<f32> NVGRenderer::get_text_size(const std::string& text,
                                             const std::string_view& font_name, const f32 font_size,
                                             const Align alignment) const
    {
        this->set_text_properties_(font_name, font_size, alignment);
        const f32 width{ nvg::text_bounds(m_nvg_context.get(), ds::point<f32>::zero(), text) };

        constexpr static f32 width_buffer{ 2.0f };
        return ds::dims{
            width + width_buffer,
            font_size,
        };
    }

    void NVGRenderer::draw_rect_outline(const ds::rect<f32>& rect, const f32 stroke_width,
                                        const ds::color<f32>& color, const Outline type) const
    {
        nvg::stroke_width(m_nvg_context.get(), stroke_width);
        nvg::begin_path(m_nvg_context.get());

        switch (type) {
            case Outline::Inner:
                nvg::rect(
                    m_nvg_context.get(),
                    ds::rect<f32>{
                        { rect.pt.x + (stroke_width / 2.0f), rect.pt.y + (stroke_width / 2.0f) },
                        { rect.size.width - stroke_width, rect.size.height - stroke_width },
                    });
                break;

            case Outline::Outer:
                nvg::rect(
                    m_nvg_context.get(),
                    ds::rect<f32>{
                        { rect.pt.x - (stroke_width / 2.0f), rect.pt.y - (stroke_width / 2.0f) },
                        { rect.size.width + stroke_width, rect.size.height + stroke_width },
                    });
                break;
        }

        nvg::stroke_color(m_nvg_context.get(), color);
        nvg::stroke(m_nvg_context.get());
    }

    void NVGRenderer::draw_rounded_rect(const ds::rect<f32>& rect, const f32 corner_radius) const
    {
        nvg::rounded_rect(m_nvg_context.get(), rect.pt.x, rect.pt.y, rect.size.width,
                          rect.size.height, corner_radius);
    }
}
