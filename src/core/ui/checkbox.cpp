#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/checkbox.hpp"
#include "core/ui/widget.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {
    using namespace vg;

    CheckBox::CheckBox(ui::Widget* parent, const std::string& caption,
                       const std::function<void(bool)>& toggled_callback)
        : ui::Widget{ parent }
        , m_caption{ caption }
        , m_toggled_callback{ toggled_callback }
    {
        // ui::Widget default value override
        m_icon_extra_scale = 1.2f;
    }

    const std::string& CheckBox::caption() const
    {
        return m_caption;
    }

    void CheckBox::set_caption(const std::string& caption)
    {
        m_caption = caption;
    }

    const bool& CheckBox::checked() const
    {
        return m_checked;
    }

    void CheckBox::set_checked(const bool& checked)
    {
        m_checked = checked;
    }

    const bool& CheckBox::pushed() const
    {
        return m_pushed;
    }

    void CheckBox::set_pushed(const bool& pushed)
    {
        m_pushed = pushed;
    }

    const std::function<void(bool)>& CheckBox::toggled_callback() const
    {
        return m_toggled_callback;
    }

    void CheckBox::set_toggled_callback(const std::function<void(bool)>& toggled_callback)
    {
        m_toggled_callback = toggled_callback;
    }

    bool CheckBox::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        ui::Widget::on_mouse_button_pressed(mouse, kb);

        if (!m_enabled)
            return false;

        if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            m_pushed = true;
            return true;
        }

        return false;
    }

    bool CheckBox::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        ui::Widget::on_mouse_button_pressed(mouse, kb);

        if (!m_enabled)
            return false;

        if (!mouse.is_button_released(Mouse::Button::Left))
            return false;

        if (m_pushed)
        {
            if (this->contains(mouse.pos()))
            {
                m_checked = !m_checked;
                if (m_toggled_callback != nullptr)
                    m_toggled_callback(m_checked);
            }

            m_pushed = false;
        }

        return true;
    }

    ds::dims<i32> CheckBox::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_fixed_size != ds::dims<i32>::zero())
            return m_fixed_size;

        nvgFontSize(nvg_context, this->font_size());
        nvgFontFace(nvg_context, font::name::sans);

        f32 text_bounds{ nvgTextBounds(nvg_context, 0, 0, m_caption.c_str(), nullptr, nullptr) };
        return ds::dims<i32>{
            static_cast<i32>(text_bounds + 1.8f * this->font_size()),
            static_cast<i32>(this->font_size() * 1.3f),
        };
    }

    void CheckBox::draw(NVGcontext* nvg_context)
    {
        ui::Widget::draw(nvg_context);

        nvgFontSize(nvg_context, this->font_size());
        nvgFontFace(nvg_context, font::name::sans);
        nvgFillColor(nvg_context,
                     m_enabled ? m_theme->m_text_color : m_theme->m_disabled_text_color);
        nvgTextAlign(nvg_context, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgText(nvg_context, m_pos.x + 1.6f * this->font_size(), m_pos.y + m_size.height * 0.5f,
                m_caption.c_str(), nullptr);

        NVGpaint bg{ nvgBoxGradient(
            nvg_context, m_pos.x + 1.5f, m_pos.y + 1.5f, m_size.height - 2.0f, m_size.height - 2.0f,
            3, 3, m_pushed ? ds::color<u8>{ 0, 0, 0, 100 } : ds::color<u8>{ 0, 0, 0, 32 },
            ds::color<u8>{ 0, 0, 0, 180 }) };

        nvgBeginPath(nvg_context);
        nvgRoundedRect(nvg_context, m_pos.x + 1.0f, m_pos.y + 1.0f, m_size.height - 2.0f,
                       m_size.height - 2.0f, 3);
        nvgFillPaint(nvg_context, bg);
        nvgFill(nvg_context);

        if (m_checked)
        {
            nvgFontSize(nvg_context, this->icon_scale() * m_size.height);
            nvgFontFace(nvg_context, font::name::icons);
            nvgFillColor(nvg_context,
                         m_enabled ? m_theme->m_icon_color : m_theme->m_disabled_text_color);
            nvgTextAlign(nvg_context, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(nvg_context, m_pos.x + m_size.height * 0.5f + 1, m_pos.y + m_size.height * 0.5f,
                    rl::utf8(m_theme->m_check_box_icon).data(), nullptr);
        }
    }
}
