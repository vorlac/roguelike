#include "core/ui/gui_canvas.hpp"
#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    PopupButton::PopupButton(ui::Widget* parent, const std::string& caption,
                             ui::Icon::ID button_icon)
        : ui::Button{ parent, caption, button_icon }
    {
        m_icon_extra_scale = 0.8f;
        m_chevron_icon = m_theme->m_popup_chevron_right_icon;
        constexpr auto popup_btn_flags{ Button::Flags(Button::ToggleButton | Button::PopupButton) };
        this->set_flags(popup_btn_flags);

        m_popup = new ui::Popup{ this->canvas(), this->dialog() };
        m_popup->set_size({ 320.0f, 250.0f });
        m_popup->set_visible(false);
    }

    void PopupButton::set_chevron_icon(ui::Icon::ID icon)
    {
        m_chevron_icon = icon;
    }

    ui::Icon::ID PopupButton::chevron_icon() const
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

    ds::dims<f32> PopupButton::preferred_size(nvg::NVGcontext* nvg_context) const
    {
        constexpr static ds::dims<f32> width_buffer{ 15.0f, 0.0f };
        return Button::preferred_size(nvg_context) + width_buffer;
    }

    void PopupButton::draw(nvg::NVGcontext* nvg_context)
    {
        if (!m_enabled && m_pressed)
            m_pressed = false;

        m_popup->set_visible(m_pressed);
        ui::Button::draw(nvg_context);

        if (m_chevron_icon != Icon::None)
        {
            const f32 text_size{ m_font_size < 0.0f ? m_theme->m_button_font_size : m_font_size };
            const std::string icon{ utf8(std::to_underlying(m_chevron_icon)) };
            const ds::color<f32> text_color{ m_text_color.a == 0.0f ? m_theme->m_text_color
                                                                    : m_text_color };
            nvg::FontFace(nvg_context, font::name::icons);
            nvg::FontSize(nvg_context, text_size * this->icon_scale());
            nvg::FillColor(nvg_context, m_enabled ? text_color : m_theme->m_disabled_text_color);
            nvg::TextAlign(nvg_context, Text::Alignment::HLeftVMiddle);

            const f32 icon_width{ nvg::TextBounds(nvg_context, 0.0f, 0.0f, icon.data(), nullptr,
                                                  nullptr) };

            ds::point<f32> icon_pos{ 0.0f, m_pos.y + m_size.height * 0.5f - 1.0f };
            if (m_popup->side() == Popup::Side::Right)
                icon_pos.x = m_pos.x + m_size.width - icon_width - 8.0f;
            else
                icon_pos.x = m_pos.x + 8.0f;

            nvg::Text(nvg_context, icon_pos.x, icon_pos.y, icon.data(), nullptr);
        }
    }

    void PopupButton::perform_layout(nvg::NVGcontext* nvg_context)
    {
        ui::Widget::perform_layout(nvg_context);

        const ui::Dialog* parent_dialog{ this->dialog() };
        if (parent_dialog != nullptr)
        {
            const f32 anchor_size{ m_popup->anchor_size() };
            const f32 pos_y{ m_pos.y - parent_dialog->position().y + (m_size.height / 2.0f) };
            if (m_popup->side() == Popup::Side::Right)
            {
                const ds::point<f32> anchor_pos{
                    parent_dialog->width() + anchor_size,
                    pos_y,
                };
                m_popup->set_anchor_pos(anchor_pos);
            }
            else
            {
                const ds::point<f32> anchor_pos{ -anchor_size, pos_y };
                m_popup->set_anchor_pos(anchor_pos);
            }
        }
        else
        {
            const f32 anchor_size{ m_popup->anchor_size() };
            const ds::point<f32> offset{
                this->width() + anchor_size + 1.0f,
                m_size.height / 2.0f - anchor_size,
            };

            const ds::point<f32> popup_pos{ this->abs_position() + offset };
            m_popup->set_position(popup_pos);
        }
    }

    void PopupButton::set_side(Popup::Side side)
    {
        const ui::Icon::ID right_icon{ m_theme->m_popup_chevron_right_icon };
        const ui::Icon::ID left_icon{ m_theme->m_popup_chevron_left_icon };

        switch (m_popup->side())
        {
            case Popup::Side::Right:
                if (m_chevron_icon == right_icon)
                    this->set_chevron_icon(left_icon);
                break;

            case Popup::Side::Left:
                if (m_chevron_icon == left_icon)
                    this->set_chevron_icon(right_icon);
                break;

            default:
                break;
        }

        m_popup->set_side(side);
    }
}
