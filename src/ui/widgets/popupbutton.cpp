#include "graphics/vg/nanovg_state.hpp"
#include "ui/canvas.hpp"
#include "ui/theme.hpp"
#include "ui/widgets/popupbutton.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    PopupButton::PopupButton(Widget* parent, std::string caption, const Icon::ID button_icon)
        : Button{ parent, std::forward<std::string>(caption), button_icon }
    {
        scoped_log();

        this->set_icon_extra_scale(0.8f);
        this->set_chevron_icon(m_theme->popup_chevron_right_icon);
        this->set_property(Property::TogglePopupMenu);

        m_popup = new Popup{
            this->canvas(),
            this->dialog(),
        };

        // TODO: why?.. necessary?
        m_popup->set_size({
            750.0f,
            300.0f,
        });

        m_popup->set_visible(false);
    }

    void PopupButton::set_chevron_icon(const Icon::ID icon)
    {
        scoped_log("{}", static_cast<i32>(icon));
        m_chevron_icon = icon;
    }

    Icon::ID PopupButton::chevron_icon() const
    {
        scoped_log("{}", static_cast<i32>(m_chevron_icon));
        return m_chevron_icon;
    }

    Side PopupButton::side() const
    {
        scoped_logger(log_level::debug, "{}", m_popup->side());
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
        scoped_trace(log_level::trace);
        constexpr static ds::dims width_buffer{ 24.0f, 0.0f };
        return Button::preferred_size() + width_buffer;
    }

    void PopupButton::draw()
    {
        scoped_trace(log_level::trace);
        if (!m_enabled && m_pressed)
            m_pressed = false;

        m_popup->set_visible(m_pressed);

        Button::draw();

        if (m_chevron_icon != Icon::None) {
            auto&& context{ m_renderer->context() };
            const f32 text_size{ m_font_size < 0.0f ? m_theme->button_font_size : m_font_size };
            const std::string icon{ utf8(m_chevron_icon) };
            const ds::color<f32>& text_color{ math::equal(m_text_color.a, 0.0f)
                                                  ? m_theme->text_color
                                                  : m_text_color };

            nvg::set_font_face(context, text::font::style::Icons);
            nvg::set_font_size(context, text_size * this->icon_scale());
            nvg::fill_color(context, m_enabled ? text_color : m_theme->disabled_text_color);
            nvg::set_text_align(context, Align::HLeft | Align::VMiddle);

            const f32 icon_width{ nvg::text_bounds(context, ds::point<f32>::zero(), icon) };
            ds::point<f32> icon_pos{ 0.0f, m_rect.pt.y + m_rect.size.height * 0.5f - 1.0f };
            if (m_popup->side() == Side::Right)
                icon_pos.x = m_rect.pt.x + m_rect.size.width - icon_width - 8.0f;
            else
                icon_pos.x = m_rect.pt.x + 8.0f;

            nvg::draw_text(context, icon_pos, icon);
        }
    }

    void PopupButton::perform_layout()
    {
        scoped_trace(log_level::trace);

        Widget::perform_layout();

        ScrollableDialog* parent_dialog{ this->dialog() };
        if (parent_dialog != nullptr) {
            const f32 anchor_size{ m_popup->anchor_size() };
            const f32 pos_y{ this->abs_position().y - parent_dialog->position().y +
                             (m_rect.size.height / 2.0f) };

            if (m_popup->side() == Side::Right) {
                const ds::point<f32> anchor_pos{
                    parent_dialog->width() + anchor_size,
                    pos_y,
                };

                m_popup->set_anchor_pos(anchor_pos);
            }
            else {
                const ds::point<f32> anchor_pos{ -anchor_size, pos_y };
                m_popup->set_anchor_pos(anchor_pos);
            }
        }
        else {
            const f32 anchor_size{ m_popup->anchor_size() };
            const ds::point<f32> offset{
                this->width() + anchor_size + 1.0f,
                (m_rect.size.height / 2.0f) - anchor_size,
            };

            m_popup->set_position(ds::point<f32>{ m_rect.pt + offset });
        }
    }

    void PopupButton::set_side(const Side side)
    {
        scoped_trace(log_level::debug);
        const Icon::ID right_icon{ m_theme->popup_chevron_right_icon };
        const Icon::ID left_icon{ m_theme->popup_chevron_left_icon };

        switch (m_popup->side()) {
            case Side::Right:
                if (m_chevron_icon == right_icon)
                    this->set_chevron_icon(left_icon);
                break;
            case Side::Left:
                if (m_chevron_icon == left_icon)
                    this->set_chevron_icon(right_icon);
                break;

            case Side::None:
            case Side::Top:
            case Side::Bottom:
            case Side::TopLeft:
            case Side::TopRight:
            case Side::BottomLeft:
            case Side::BottomRight:
            default:
                break;
        }

        m_popup->set_side(side);
    }
}
