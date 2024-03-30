#include <memory>

#include "core/ui/theme.hpp"
#include "core/ui/widgets/label.hpp"
#include "ds/dims.hpp"
#include "ds/shared.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl::ui {

    Label::Label(std::string text, const std::string_view& font, const f32 font_size)
        : Widget{ nullptr }
        , m_text{ std::move(text) }
        , m_font{ font }
    {
        m_font_size = font_size;
    }

    Label::Label(Widget* parent, std::string text, const std::string_view& font, const f32 font_size)
        : Widget{ parent }
        , m_text{ std::move(text) }
        , m_font{ font }
    {
        if (m_theme != nullptr)
        {
            m_font_size = m_theme->standard_font_size;
            m_color = m_theme->text_color;
        }

        if (font_size >= 0.0f)
            m_font_size = font_size;
    }

    const std::string& Label::text() const
    {
        return m_text;
    }

    const std::string& Label::font() const
    {
        return m_font;
    }

    const ds::color<f32>& Label::color() const
    {
        return m_color;
    }

    void Label::set_text(const std::string& text)
    {
        m_text = text;
    }

    void Label::set_font(const std::string& font)
    {
        m_font = font;
    }

    void Label::set_color(const ds::color<f32>& color)
    {
        m_color = color;
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
            m_font_size = m_theme->form_label_font_size;
            m_color = m_theme->text_color;
        }
    }

    ds::dims<f32> Label::preferred_size() const
    {
        if (m_text.empty())
            return { 0.0f, 0.0f };

        const auto context{ m_renderer->context() };
        m_renderer->set_text_properties_(m_theme->form_label_font_name,
                                         m_theme->form_label_font_size,
                                         nvg::Align::HLeft | nvg::Align::VMiddle);

        if (m_fixed_size.width > 0.0f)
        {
            std::array<f32, 4> bounds{ 0.0f };
            nvg::set_text_align(context, nvg::Align::HLeft | nvg::Align::VTop);
            nvg::text_box_bounds_(context, m_rect.pt.x, m_rect.pt.y, m_fixed_size.width,
                                  m_text.c_str(), nullptr, bounds.data());

            const f32 textbox_height{ bounds[3] - bounds[1] };
            return ds::dims{
                m_fixed_size.width,
                textbox_height,
            };
        }

        nvg::set_text_align(context, nvg::Align::HLeft | nvg::Align::VMiddle);
        const f32 text_width{ nvg::text_bounds_(context, 0.0f, 0.0f, m_text.c_str()) };
        return {
            text_width + 2.0f,
            this->font_size(),
        };
    }

    void Label::draw()
    {
        ui::Widget::draw();

        auto context{ m_renderer->context() };
        nvg::set_font_face(context, m_font);
        nvg::set_font_size(context, this->font_size());
        nvg::fill_color(context, m_color);

        if (m_fixed_size.width > 0)
        {
            nvg::set_text_align(context, nvg::Align::HLeft | nvg::Align::VTop);
            nvg::text_box_(context, m_rect.pt.x, m_rect.pt.y, m_fixed_size.width, m_text.c_str());
        }
        else
        {
            nvg::set_text_align(context, nvg::Align::HLeft | nvg::Align::VMiddle);
            nvg::text_(context, m_rect.pt.x, m_rect.pt.y + m_rect.size.height * 0.5f,
                       m_text.c_str());
        }
    }
}
