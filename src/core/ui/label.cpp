#include "core/ui/label.hpp"
#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/shared.hpp"
#include "graphics/vg/nanovg.hpp"
#include "resources/fonts.hpp"

namespace rl::ui {
    using namespace vg;

    Label::Label(ui::Widget* parent, const std::string& caption, const std::string& font,
                 f32 font_size)
        : ui::Widget{ parent }
        , m_caption{ caption }
        , m_font{ font }
    {
        if (m_theme != nullptr)
        {
            m_font_size = m_theme->m_standard_font_size;
            m_color = m_theme->m_text_color;
        }

        if (font_size >= 0)
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

    void Label::set_color(ds::color<u8> color)
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
            m_font_size = m_theme->m_standard_font_size;
            m_color = m_theme->m_text_color;
        }
    }

    ds::dims<f32> Label::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_caption.empty())
            return ds::dims<f32>::zero();

        nvgFontFace(nvg_context, m_font.c_str());
        nvgFontSize(nvg_context, this->font_size());

        if (m_fixed_size.width > 0)
        {
            f32 bounds[4] = { 0 };
            nvgTextAlign(nvg_context, Text::Alignment::TopLeft);
            nvgTextBoxBounds(nvg_context, m_pos.x, m_pos.y, m_fixed_size.width, m_caption.c_str(),
                             nullptr, bounds);

            return ds::dims<f32>{
                m_fixed_size.width,
                bounds[3] - bounds[1],
            };
        }
        else
        {
            nvgTextAlign(nvg_context, Text::Alignment::CenteredLeft);

            return ds::dims<f32>{
                nvgTextBounds(nvg_context, 0.0f, 0.0f, m_caption.c_str(), nullptr, nullptr) + 2.0f,
                this->font_size(),
            };
        }
    }

    void Label::draw(NVGcontext* nvg_context)
    {
        ui::Widget::draw(nvg_context);

        nvgFontFace(nvg_context, m_font.c_str());
        nvgFontSize(nvg_context, this->font_size());
        nvgFillColor(nvg_context, m_color);

        if (m_fixed_size.width > 0)
        {
            nvgTextAlign(nvg_context, Text::Alignment::TopLeft);
            nvgTextBox(nvg_context, m_pos.x, m_pos.y, m_fixed_size.width, m_caption.c_str(),
                       nullptr);
        }
        else
        {
            nvgTextAlign(nvg_context, Text::Alignment::CenteredLeft);
            nvgText(nvg_context, m_pos.x, m_pos.y + m_size.height * 0.5f, m_caption.c_str(),
                    nullptr);
        }
    }
}
