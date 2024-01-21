#include "core/ui/label.hpp"
#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/shared.hpp"
#include "graphics/vg/nanovg.hpp"
#include "resources/fonts.hpp"

namespace rl::ui {

    Label::Label(ui::Widget* parent, const std::string& caption, const std::string& font,
                 f32 font_size)
        : ui::Widget{ parent }
        , m_caption{ caption }
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

    std::string Label::caption() const
    {
        return m_caption;
    }

    std::string Label::font() const
    {
        return m_font;
    }

    ds::color<f32> Label::color() const
    {
        return m_color;
    }

    void Label::set_caption(const std::string& caption)
    {
        m_caption = caption;
    }

    void Label::set_font(const std::string& font)
    {
        m_font = font;
    }

    void Label::set_color(ds::color<f32> color)
    {
        m_color = color;
    }

    void Label::set_callback(std::function<void()> callable)
    {
        m_callback = callable;
    }

    void Label::set_theme(ui::Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme != nullptr)
        {
            m_font_size = m_theme->standard_font_size;
            m_color = m_theme->text_color;
        }
    }

    ds::dims<f32> Label::preferred_size() const
    {
        if (m_caption.empty())
            return ds::dims<f32>::zero();

        auto&& context{ m_renderer->context() };
        nvg::FontFace(context, m_font.c_str());
        nvg::FontSize(context, this->font_size());

        if (m_fixed_size.width > 0.0f)
        {
            std::array<f32, 4> bounds{ 0.0f };
            nvg::TextAlign(context, Text::Alignment::HLeftVTop);
            nvg::TextBoxBounds(context, m_pos.x, m_pos.y, m_fixed_size.width, m_caption.c_str(),
                               nullptr, bounds.data());

            const f32 textbox_height{ bounds[3] - bounds[1] };
            return ds::dims<f32>{
                m_fixed_size.width,
                textbox_height,
            };
        }
        else
        {
            nvg::TextAlign(context, Text::Alignment::HLeftVMiddle);
            const f32 text_width{ nvg::TextBounds(context, 0.0f, 0.0f, m_caption.c_str()) };
            return ds::dims<f32>{
                text_width + 2.0f,
                this->font_size(),
            };
        }
    }

    void Label::draw()
    {
        ui::Widget::draw();

        auto&& context{ m_renderer->context() };
        nvg::FontFace(context, m_font.c_str());
        nvg::FontSize(context, this->font_size());
        nvg::FillColor(context, m_color);

        if (m_fixed_size.width > 0)
        {
            nvg::TextAlign(context, Text::Alignment::HLeftVTop);
            nvg::TextBox(context, m_pos.x, m_pos.y, m_fixed_size.width, m_caption.c_str());
        }
        else
        {
            nvg::TextAlign(context, Text::Alignment::HLeftVMiddle);
            nvg::Text(context, m_pos.x, m_pos.y + m_size.height * 0.5f, m_caption.c_str());
        }
    }
}
