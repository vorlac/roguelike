#include <nanovg.h>

#include "core/ui/label.hpp"
#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/shared.hpp"
#include "resources/fonts.hpp"

namespace rl::ui {

    label::label(ui::widget* parent, const std::string& caption, const std::string& font,
                 i32 font_size)
        : ui::widget{ parent }
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

    std::string label::caption() const
    {
        return m_caption;
    }

    std::string label::font() const
    {
        return m_font;
    }

    ds::color<f32> label::color() const
    {
        return m_color;
    }

    void label::set_caption(const std::string& caption)
    {
        m_caption = caption;
    }

    void label::set_font(const std::string& font)
    {
        m_font = font;
    }

    void label::set_color(ds::color<f32> color)
    {
        m_color = color;
    }

    void label::set_callback(std::function<void()> callable)
    {
        m_callback = callable;
    }

    void label::set_theme(ui::theme* theme)
    {
        widget::set_theme(theme);
        if (m_theme != nullptr)
        {
            m_font_size = m_theme->m_standard_font_size;
            m_color = m_theme->m_text_color;
        }
    }

    ds::dims<i32> label::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_caption.empty())
            return ds::dims<i32>{ 0, 0 };

        nvgFontFace(nvg_context, m_font.c_str());
        nvgFontSize(nvg_context, this->font_size());

        if (m_fixed_size.width > 0)
        {
            float bounds[4];
            nvgTextAlign(nvg_context, NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_TOP);
            nvgTextBoxBounds(nvg_context, m_pos.x, m_pos.y, m_fixed_size.width, m_caption.c_str(),
                             nullptr, bounds);
            return ds::dims<i32>(m_fixed_size.width, bounds[3] - bounds[1]);
        }
        else
        {
            nvgTextAlign(nvg_context, NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_MIDDLE);
            return ds::dims<i32>(
                nvgTextBounds(nvg_context, 0, 0, m_caption.c_str(), nullptr, nullptr) + 2,
                font_size());
        }
    }

    void label::draw(NVGcontext* nvg_context)
    {
        ui::widget::draw(nvg_context);

        nvgFontFace(nvg_context, m_font.c_str());
        nvgFontSize(nvg_context, font_size());
        nvgFillColor(nvg_context, m_color);

        if (m_fixed_size.width > 0)
        {
            nvgTextAlign(nvg_context, NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_TOP);
            nvgTextBox(nvg_context, m_pos.x, m_pos.y, m_fixed_size.width, m_caption.c_str(),
                       nullptr);
        }
        else
        {
            nvgTextAlign(nvg_context, NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_MIDDLE);
            nvgText(nvg_context, m_pos.x, m_pos.y + m_size.height * 0.5f, m_caption.c_str(),
                    nullptr);
        }
    }
}
