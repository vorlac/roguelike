#include <glad/gl.h>

#include <bit>
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
        const u8 float_mode{ 0 };
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
        nvg_flags |= Property::Debug;

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
            //    ui::Font::name::sans,
            //    std::basic_string_view<u8>{
            //        roboto_regular_ttf,
            //        roboto_regular_ttf_size,
            //    },
            //},
            //{
            //    ui::Font::name::sans_bold,
            //    std::basic_string_view<u8>{
            //        roboto_bold_ttf,
            //        roboto_bold_ttf_size,
            //    },
            //},
            //{
            //    ui::Font::name::mono,
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

    void NVGRenderer::begin_frame(const ds::dims<f32>& render_size, const f32 pixel_ratio) const
    {
        nvg::begin_frame(m_nvg_context, render_size.width, render_size.height, pixel_ratio);
    }

    void NVGRenderer::end_frame() const
    {
        nvg::end_frame(m_nvg_context);
    }

    void NVGRenderer::save_state() const
    {
        nvg::save(m_nvg_context);
    }

    void NVGRenderer::restore_state() const
    {
        nvg::restore(m_nvg_context);
    }

    ui::Font::ID NVGRenderer::load_font(const std::string_view& font_name,
                                        const std::u8string_view& font_ttf) const
    {
        return nvg::create_font_mem(m_nvg_context, font_name, (u8*)font_ttf.data());
    }

    void NVGRenderer::flush(const ds::dims<f32>& viewport, const f32 pixel_ratio) const
    {
        // Flush all queued up NanoVG rendering commands
        const nvg::NVGparams* params{ nvg::internal_params(m_nvg_context) };
        params->renderFlush(params->userPtr);
        params->renderViewport(params->userPtr, static_cast<f32>(viewport.width),
                               static_cast<f32>(viewport.height), pixel_ratio);
    }

    void NVGRenderer::load_fonts(const std::vector<FontInfo>& fonts)
    {
        for (auto&& font_info : fonts)
        {
            auto&& [font_name, font_ttf] = font_info;
            const auto handle{ this->load_font(font_name, font_ttf) };
            m_font_map[font_name] = handle;
        }
    }

    void NVGRenderer::set_text_properties(const std::string_view& font_name, const f32 font_size,
                                          const ui::Text::Alignment alignment) const
    {
        nvg::font_face(m_nvg_context, font_name.data());
        nvg::font_size(m_nvg_context, font_size);
        nvg::text_align(m_nvg_context, alignment);
    }

    ds::dims<f32> NVGRenderer::get_text_size(const std::string& text) const
    {
        assert_cond(false);
        return {};
    }

    ds::rect<f32> NVGRenderer::get_text_box_rect(
        const std::string& text, const ds::point<f32>& pos, const std::string_view& font_name,
        const f32 font_size, const f32 fold_width, const ui::Text::Alignment alignment) const
    {
        this->set_text_properties(font_name, font_size, alignment);

        std::array<f32, 4> bounds{ 0.0f };
        nvg::text_box_bounds(m_nvg_context, pos.x, pos.y, fold_width, text.data(), nullptr,
                             &bounds.front());

        return ds::rect<f32>{
            pos,
            ds::dims{
                fold_width,
                bounds[3] - bounds[1],
            },
        };
    }

    ds::dims<f32> NVGRenderer::get_text_size(const std::string& text,
                                             const std::string_view& font_name, const f32 font_size,
                                             const ui::Text::Alignment alignment) const
    {
        this->set_text_properties(font_name, font_size, alignment);

        const f32 width{ nvg::text_bounds(m_nvg_context, 0.0f, 0.0f, text.data(), nullptr,
                                          nullptr) };

        constexpr static f32 width_buffer{ 2.0f };
        return ds::dims<f32>{
            width + width_buffer,
            font_size,
        };
    }

    void NVGRenderer::draw_rect_outline(const ds::rect<f32>& rect, const f32 stroke_width,
                                        const ds::color<f32>& color, const ui::Outline type) const
    {
        nvg::stroke_width(m_nvg_context, stroke_width);
        nvg::begin_path(m_nvg_context);

        switch (type)
        {
            case ui::Outline::Inner:
                nvg::rect(m_nvg_context, rect.pt.x + (stroke_width / 2.0f),
                          rect.pt.y + (stroke_width / 2.0f), rect.size.width - stroke_width,
                          rect.size.height - stroke_width);
                break;
            case ui::Outline::Outer:
                nvg::rect(m_nvg_context, rect.pt.x - (stroke_width / 2.0f),
                          rect.pt.y - (stroke_width / 2.0f), rect.size.width + stroke_width,
                          rect.size.height + stroke_width);
                break;
        }

        nvg::stroke_color(m_nvg_context, color.nvg());
        nvg::stroke(m_nvg_context);
    }
}
