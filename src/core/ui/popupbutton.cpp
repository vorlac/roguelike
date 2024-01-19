#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {
    using namespace vg;

    PopupButton::PopupButton(ui::Widget* parent, const std::string& caption,
                             ui::Icon::ID button_icon)
        : ui::Button{ parent, caption, button_icon }
    {
        m_chevron_icon = m_theme->m_popup_chevron_right_icon;
        auto popup_btn_flags{ Button::Flags(Flags::ToggleButton | Flags::PopupButton) };
        this->set_flags(popup_btn_flags);

        m_popup = new ui::Popup{ this, this->dialog() };
        m_popup->set_size({ 320, 250 });
        m_popup->set_visible(false);

        m_icon_extra_scale = 0.8f;  // widget override
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

    ds::dims<f32> PopupButton::preferred_size(NVGcontext* ctx) const
    {
        return Button::preferred_size(ctx) + ds::dims<f32>{ 15.0f, 0.0f };
    }

    void PopupButton::draw(NVGcontext* ctx)
    {
        if (!m_enabled && m_pressed)
            m_pressed = false;

        m_popup->set_visible(m_pressed);

        ui::Button::draw(ctx);

        if (m_chevron_icon != Icon::None)
        {
            std::string icon{ utf8(std::to_underlying(m_chevron_icon)) };
            ds::color<u8> text_color{ m_text_color.a == 0 ? m_theme->m_text_color : m_text_color };
            f32 text_size{ m_font_size < 0.0f ? m_theme->m_button_font_size : m_font_size };

            nvgFontFace(ctx, font::name::icons);
            nvgFontSize(ctx, text_size * this->icon_scale());
            nvgFillColor(ctx, m_enabled ? text_color : m_theme->m_disabled_text_color);
            nvgTextAlign(ctx, NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_MIDDLE);

            f32 iw{ nvgTextBounds(ctx, 0.0f, 0.0f, icon.data(), nullptr, nullptr) };
            ds::point<f32> icon_pos{
                0.0f,
                m_pos.y + m_size.height * 0.5f - 1.0f,
            };

            if (m_popup->side() == Popup::Side::Right)
                icon_pos.x = m_pos.x + m_size.width - iw - 8;
            else
                icon_pos.x = m_pos.x + 8;

            nvgText(ctx, icon_pos.x, icon_pos.y, icon.data(), nullptr);
        }
    }

    void PopupButton::perform_layout(NVGcontext* ctx)
    {
        ui::Widget::perform_layout(ctx);
        f32 anchor_size{ m_popup->anchor_size() };

        const ui::Dialog* parent_dialog{ this->dialog() };
        if (parent_dialog != nullptr)
        {
            f32 pos_y{ this->abs_position().y - parent_dialog->position().y + m_size.height / 2 };
            if (m_popup->side() == Popup::Side::Right)
                m_popup->set_anchor_pos(ds::point<f32>{
                    parent_dialog->width() + anchor_size,
                    pos_y,
                });
            else
                m_popup->set_anchor_pos(ds::point<f32>{
                    -anchor_size,
                    pos_y,
                });
        }
        else
        {
            m_popup->set_position(this->abs_position() + ds::point<f32>{
                                                             this->width() + anchor_size + 1,
                                                             m_size.height / 2 - anchor_size,
                                                         });
        }
    }

    void PopupButton::set_side(Popup::Side side)
    {
        if (m_popup->side() == Popup::Side::Right &&
            m_chevron_icon == m_theme->m_popup_chevron_right_icon)
            set_chevron_icon(m_theme->m_popup_chevron_left_icon);
        else if (m_popup->side() == Popup::Side::Left &&
                 m_chevron_icon == m_theme->m_popup_chevron_left_icon)
            set_chevron_icon(m_theme->m_popup_chevron_right_icon);

        m_popup->set_side(side);
    }
}
