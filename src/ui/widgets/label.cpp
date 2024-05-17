#include <memory>

#include "ds/dims.hpp"
#include "gfx/vg/nanovg.hpp"
#include "ui/theme.hpp"
#include "ui/widgets/label.hpp"

namespace rl::ui {
    Label::Label(std::string text, const f32 font_size, const Align alignment)
        : Label{ nullptr, std::move(text), font_size, alignment }
    {
    }

    Label::Label(Widget* parent, std::string text, const f32 font_size, const Align alignment)
        : Widget{ parent }
        , m_text{ std::move(text) }
        , m_text_alignment{ alignment }
    {
        m_font_size = m_theme->label_font_size;
        if (font_size >= text::font::MinValidSize)
            m_font_size = font_size;

        if (alignment != Align::None)
            m_text_alignment = alignment;
    }

    void Label::set_theme(Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme != nullptr) {
            if (!this->has_font_size_override())
                m_font_size = m_theme->label_font_size;

            // TODO: handle color overrides?
            m_text_color = m_theme->label_font_color;
            m_text_outline_color = m_theme->text_shadow_color;
        }
    }

    ds::dims<f32> Label::preferred_size() const
    {
        if (m_text.empty())
            return ds::dims<f32>::zero();

        debug_assert(Align::None != m_text_alignment,
                     "invalid text alignment value assigned in label");

        const auto context{ m_renderer->context() };
        const bool is_fixed_size{ math::not_equal(m_fixed_size.width, 0.0f) &&
                                  m_fixed_size.width > 0.0f };

        m_renderer->set_text_properties(m_font, m_font_size, m_text_alignment);
        if (is_fixed_size || (m_font_autosizing && !m_rect.contained_by(this->parent()->rect()))) {
            // using TL aligntment since the font size will be computed from the predefined width
            nvg::set_text_align(context, Align::HLeft | Align::VTop);

            if (is_fixed_size) {
                const ds::rect<f32> bounds{ nvg::text_box_bounds(
                    context, m_rect.pt, m_fixed_size.width, m_text) };

                return ds::dims{ m_fixed_size.width, bounds.size.height };
            }

            const ds::rect<f32> prect{ this->parent()->rect() };
            const ds::rect<f32> bounds{ nvg::text_box_bounds(
                context, prect.pt, prect.size.width + 2.0f, m_text) };

            return ds::dims{ bounds.size.width, bounds.size.height };
        }

        nvg::set_text_align(context, m_text_alignment);
        const f32 text_width{ nvg::text_bounds(context, ds::point<f32>{}, m_text) };
        return ds::dims{ text_width + 2.0f, m_font_size };
    }

    void Label::draw()
    {
        Widget::draw();

        m_renderer->set_text_properties(m_font, m_font_size, m_text_alignment);

        const auto context{ m_renderer->context() };
        if (math::not_equal(m_fixed_size.width, 0.0f) && m_fixed_size.width > 0.0f) {
            // use TL aligntment if nvg has to compute
            // the font size from a predefined width
            nvg::fill_color(context, m_text_color);
            nvg::set_text_align(context, Align::HLeft | Align::VTop);
            nvg::text_box(context, m_rect.pt, m_fixed_size.width + 2.0f, m_text);
        }
        else {
            const ds::point<f32> pos{ m_rect.reference_point(m_text_alignment) };

            // text shadow
            nvg::font_blur_(context, 5.0f);
            nvg::fill_color(context, m_text_outline_color);
            nvg::draw_text(context, pos + 2.0f, m_text);

            // text, slightly offset from shadow pos
            nvg::font_blur_(context, 0.0f);
            nvg::fill_color(context, m_text_color);
            nvg::draw_text(context, pos + 1.0f, m_text);
        }
    }

    std::string_view Label::text() const
    {
        return m_text;
    }

    std::string_view Label::font() const
    {
        return m_font;
    }

    ds::color<f32> Label::color() const
    {
        return m_text_color;
    }

    Align Label::text_alignment() const
    {
        return m_text_alignment;
    }

    void Label::set_text(std::string text)
    {
        m_text = std::move(text);
    }

    void Label::set_font(const std::string_view font)
    {
        m_font = std::string{ font };
    }

    void Label::set_text_alignment(const Align alignment)
    {
        m_text_alignment = alignment;
    }

    void Label::set_color(const ds::color<f32> color)
    {
        m_text_color = color;
    }

    void Label::set_callback(const std::function<void()>& callable)
    {
        m_callback = callable;
    }
}
