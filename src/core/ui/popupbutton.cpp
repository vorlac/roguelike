#include "core/ui/canvas.hpp"
#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "utils/logging.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    PopupButton::PopupButton(Widget* parent, std::string caption, Icon::ID button_icon)
        : Button{ parent, caption, button_icon }
    {
        scoped_log();
        this->set_icon_extra_scale(1.0f);
        this->set_chevron_icon(m_theme->popup_chevron_right_icon);
        this->set_property(Property::TogglePopupMenu);

        m_popup = new Popup{
            this->canvas(),
            this->dialog(),
        };

        m_popup->set_visible(false);
        // TODO: why?.. necessary?
        m_popup->set_size(ds::dims<f32>{
            750.0f,
            300.0f,
        });
    }

    void PopupButton::set_chevron_icon(Icon::ID icon)
    {
        scoped_log("{}", static_cast<i32>(icon));
        m_chevron_icon = icon;
    }

    Icon::ID PopupButton::chevron_icon() const
    {
        scoped_log("{}", static_cast<i32>(m_chevron_icon));
        return m_chevron_icon;
    }

    Popup::Side PopupButton::side() const
    {
        scoped_log("{}", m_popup->side());
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

    ds::dims<f32> PopupButton::preferred_size() const
    {
        scoped_log();
        constexpr static ds::dims<f32> width_buffer{ 24.0f, 0.0f };
        return Button::preferred_size() + width_buffer;
    }

    void PopupButton::draw()
    {
        scoped_log();
        if (!m_enabled && m_pressed)
            m_pressed = false;

        m_popup->set_visible(m_pressed);
        Button::draw();

        if (m_chevron_icon != Icon::None)
        {
            const f32 text_size{ m_font_size < 0.0f ? m_theme->button_font_size : m_font_size };
            const std::string icon{ utf8(std::to_underlying(m_chevron_icon)) };
            const ds::color<f32>& text_color{ m_text_color.a == 0.0f ? m_theme->text_color
                                                                     : m_text_color };
            auto&& context{ m_renderer->context() };
            nvg::FontFace(context, font::name::icons);
            nvg::FontSize(context, text_size * this->icon_scale());
            nvg::FillColor(context, m_enabled ? text_color : m_theme->disabled_text_color);
            nvg::TextAlign(context, Text::Alignment::HLeftVMiddle);

            const f32 icon_width{ nvg::TextBounds(context, 0.0f, 0.0f, icon.data(), nullptr,
                                                  nullptr) };

            ds::point<f32> icon_pos{ 0.0f, m_pos.y + m_size.height * 0.5f - 1.0f };
            if (m_popup->side() == Popup::Side::Right)
                icon_pos.x = m_pos.x + m_size.width - icon_width - 8.0f;
            else
                icon_pos.x = m_pos.x + 8.0f;

            nvg::Text(context, icon_pos.x, icon_pos.y, icon.data(), nullptr);
        }
    }

    void PopupButton::perform_layout()
    {
        scoped_log();
        Widget::perform_layout();

        const Dialog* parent_dialog{ this->dialog() };
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
            ds::point<f32> offset{
                this->width() + anchor_size + 1.0f,
                (m_size.height / 2.0f) - anchor_size,
            };

            m_popup->set_position({ this->position() + offset });
        }
    }

    void PopupButton::set_side(Popup::Side side)
    {
        scoped_log();
        const Icon::ID right_icon{ m_theme->popup_chevron_right_icon };
        const Icon::ID left_icon{ m_theme->popup_chevron_left_icon };

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
