#include <memory>

#include "core/ui/theme.hpp"
#include "core/ui/widgets/label.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl::ui {

    Label::Label(std::string text, const f32 font_size /*= -1.0f*/,
                 const Align alignment /*= Align::HLeft | Align::VMiddle*/)
        : Label{ nullptr, std::move(text), font_size, alignment }
    {
    }

    Label::Label(Widget* parent, std::string text, const f32 font_size /*= font::InvalidSize*/,
                 const Align alignment /*= Align::HLeft | Align::VMiddle*/)
        : Widget{ parent }
        , m_text{ std::move(text) }
        , m_text_alignment{ alignment }

    {
        m_text_color = m_theme->label_font_color;
        m_text_font = m_theme->label_font_name;
        m_font_size = m_theme->label_font_size;

        if (math::not_equal(m_font_size, text::font::InvalidSize))
            m_font_size = font_size;

        if (alignment != Align::None)
            m_text_alignment = alignment;
    }

    const std::string& Label::text() const
    {
        return m_text;
    }

    const std::string& Label::font() const
    {
        return m_text_font;
    }

    const ds::color<f32>& Label::color() const
    {
        return m_text_color;
    }

    Align Label::text_alignment() const
    {
        return m_text_alignment;
    }

    void Label::set_text(const std::string& text)
    {
        m_text = text;
    }

    void Label::set_font(const std::string& font)
    {
        m_text_font = font;
    }

    void Label::set_text_alignment(const Align alignment)
    {
        m_text_alignment = alignment;
    }

    void Label::set_color(const ds::color<f32>& color)
    {
        m_text_color = color;
    }

    void Label::set_callback(const std::function<void()>& callable)
    {
        m_callback = callable;
    }

    void Label::set_theme(ui::Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme != nullptr)
        {
            m_font_size = m_theme->label_font_size;
            m_text_color = m_theme->label_font_color;
        }
    }

    ds::dims<f32> Label::preferred_size() const
    {
        if (m_text.empty())
            return ds::dims<f32>::zero();

        const auto context{ m_renderer->context() };
        runtime_assert(Align::None != m_text_alignment,
                       "invalid text alignment value assigned in label");

        m_renderer->set_text_properties_(m_text_font, m_font_size, m_text_alignment);

        bool is_fixed_size{ math::not_equal(m_fixed_size.width, 0.0f) && m_fixed_size.width > 0.0f };
        if (is_fixed_size || (m_font_autosizing && !m_rect.contained_by(this->parent()->rect())))
        {
            ds::dims<f32> size{ ds::dims<f32>::zero() };
            ds::rect<f32> bounds{ ds::rect<f32>::zero() };

            // use TL aligntment if nvg has to compute the actual font size from a predefined width
            constexpr static Align TOP_LEFT_ALIGNMENT{ Align::HLeft | Align::VTop };
            nvg::set_text_align(context, TOP_LEFT_ALIGNMENT);
            if (is_fixed_size)
            {
                bounds = nvg::text_box_bounds(context, m_rect.pt, m_fixed_size.width, m_text);
                size = ds::dims{ m_fixed_size.width, bounds.size.height };
            }
            else
            {
                ds::rect<f32> prect{ this->parent()->rect() };
                bounds = nvg::text_box_bounds(context, prect.pt, prect.size.width + 2.0f, m_text);
                // assert_cond(bounds.contained_by(prect));
                size = ds::dims{ bounds.size.width, bounds.size.height };
            }

            return size;
        }

        nvg::set_text_align(context, m_text_alignment);
        const f32 text_width{ nvg::text_bounds(context, ds::point<f32>::zero(), m_text) };
        return ds::dims{ text_width + 2.0f, m_font_size };
    }

    void Label::draw()
    {
        ui::Widget::draw();

        m_renderer->set_text_properties_(m_text_font, m_font_size, m_text_alignment);

        auto context{ m_renderer->context() };
        if (math::not_equal(m_fixed_size.width, 0.0f) && m_fixed_size.width > 0.0f)
        {
            // TODO: clean this up
            // use TL aligntment if nvg has to compute the actual font size from a predefined width
            constexpr static Align TOP_LEFT_ALIGNMENT{ Align::HLeft | Align::VTop };

            nvg::fill_color(context, m_text_color);
            nvg::set_text_align(context, TOP_LEFT_ALIGNMENT);
            nvg::text_box(context, m_rect.pt, m_fixed_size.width + 2.0f, m_text);
        }
        else
        {
            nvg::fill_color(context, m_text_color);
            // TODO: set as constexpr defautl valye
            const ds::point<f32>& pos{
                m_text_alignment == (Align::HLeft | Align::VMiddle)  //
                    ? m_rect.pt
                    : m_rect.centroid(),
            };
            nvg::set_text_align(context, m_text_alignment);
            nvg::draw_text(context, pos, m_text);
        }
    }
}
