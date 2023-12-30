#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    PopupButton::PopupButton(ui::widget* parent, const std::string& caption, i32 button_icon)
        : Button{ parent, caption, button_icon }
    {
        m_chevron_icon = m_theme->m_popup_chevron_right_icon;
        this->set_flags(Button::Flags(Flags::ToggleButton | Flags::PopupButton));

        m_popup = new Popup{ this, this->window() };
        m_popup->set_size({ 320, 250 });
        m_popup->set_visible(false);

        m_icon_extra_scale = 0.8f;  // widget override
    }

    void PopupButton::set_chevron_icon(i32 icon)
    {
        m_chevron_icon = icon;
    }

    i32 PopupButton::chevron_icon() const
    {
        return m_chevron_icon;
    }

    Popup::Side PopupButton::side() const
    {
        return m_popup->side();
    }

    Popup* PopupButton::popup()
    {
        return m_popup;
    }

    const Popup* PopupButton::popup() const
    {
        return m_popup;
    }

    ds::dims<i32> PopupButton::preferred_size(NVGcontext* ctx) const
    {
        return Button::preferred_size(ctx) + ds::dims<i32>{ 15, 0 };
    }

    void PopupButton::draw(NVGcontext* ctx)
    {
        if (!m_enabled && m_pressed)
            m_pressed = false;

        m_popup->set_visible(m_pressed);
        Button::draw(ctx);

        if (m_chevron_icon)
        {
            std::string icon{ utf8(m_chevron_icon) };
            NVGcolor text_color = m_text_color.a == 0 ? m_theme->m_text_color : m_text_color;

            nvgFontSize(ctx, (m_font_size < 0 ? m_theme->m_button_font_size : m_font_size) *
                                 this->icon_scale());
            nvgFontFace(ctx, "icons");
            nvgFillColor(ctx, m_enabled ? text_color : m_theme->m_disabled_text_color);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

            float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
            ds::point<f32> icon_pos{
                0.0f,
                m_pos.y + m_size.height * 0.5f - 1.0f,
            };

            if (m_popup->side() == Popup::Right)
                icon_pos.x = m_pos.x + m_size.width - iw - 8;
            else
                icon_pos.x = m_pos.x + 8;

            nvgText(ctx, icon_pos.x, icon_pos.y, icon.data(), nullptr);
        }
    }

    void PopupButton::perform_layout(NVGcontext* ctx)
    {
        ui::widget::perform_layout(ctx);
        i32 anchor_size{ m_popup->anchor_size() };

        const rl::Window* parent_window{ this->window() };
        if (parent_window != nullptr)
        {
            i32 pos_y{ this->abs_position().y - parent_window->position().y + m_size.height / 2 };
            if (m_popup->side() == Popup::Right)
                m_popup->set_anchor_pos(
                    ds::point<i32>{ parent_window->width() + anchor_size, pos_y });
            else
                m_popup->set_anchor_pos(ds::point<i32>{ -anchor_size, pos_y });
        }
        else
        {
            m_popup->set_position(this->abs_position() + ds::point<i32>{
                                                             this->width() + anchor_size + 1,
                                                             m_size.height / 2 - anchor_size,
                                                         });
        }
    }

    void PopupButton::set_side(Popup::Side side)
    {
        if (m_popup->side() == Popup::Right && m_chevron_icon == m_theme->m_popup_chevron_right_icon)
            set_chevron_icon(m_theme->m_popup_chevron_left_icon);
        else if (m_popup->side() == Popup::Left &&
                 m_chevron_icon == m_theme->m_popup_chevron_left_icon)
            set_chevron_icon(m_theme->m_popup_chevron_right_icon);
        m_popup->set_side(side);
    }
}
