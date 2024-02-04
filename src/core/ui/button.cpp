#include <string>
#include <string_view>
#include <utility>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/button.hpp"
#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    Button::Button(Widget* parent, std::string text, const Icon::ID icon)
        : Widget{ parent }
        , m_text{ std::forward<std::string>(text) }
        , m_icon{ icon }
    {
        if (m_theme != nullptr)
        {
            m_background_color = m_theme->button_gradient_top_focused;
            m_text_color = m_theme->text_color;
        }
    }

    bool Button::has_property(const Button::Property prop) const
    {
        return (std::to_underlying(m_props) & std::to_underlying(prop)) != 0;
    }

    void Button::set_property(const Button::Property prop)
    {
        m_props = prop;
    }

    Button::Property Button::properties() const
    {
        return m_props;
    }

    const std::string& Button::caption() const
    {
        return m_text;
    }

    void Button::set_caption(const std::string& caption)
    {
        m_text = caption;
    }

    ds::color<f32> Button::background_color() const
    {
        return m_background_color;
    }

    void Button::set_background_color(const ds::color<f32>& bg_color)
    {
        m_background_color = bg_color;
    }

    ds::color<f32> Button::text_color() const
    {
        return m_text_color;
    }

    void Button::set_text_color(const ds::color<f32>& text_color)
    {
        m_text_color = text_color;
    }

    Icon::ID Button::icon() const
    {
        return m_icon;
    }

    void Button::set_icon(const Icon::ID icon)
    {
        m_icon = icon;
    }

    Icon::Placement Button::icon_placement() const
    {
        return m_icon_placement;
    }

    void Button::set_icon_placement(const Icon::Placement placement)
    {
        m_icon_placement = placement;
    }

    bool Button::pressed() const
    {
        return m_pressed;
    }

    void Button::set_pressed(const bool pressed)
    {
        m_pressed = pressed;
    }

    const std::function<void()>& Button::callback() const
    {
        return m_callback;
    }

    void Button::set_callback(const std::function<void()>& callback)
    {
        m_callback = callback;
    }

    const std::function<void(bool)>& Button::change_callback() const
    {
        return m_change_callback;
    }

    void Button::set_change_callback(const std::function<void(bool)>& callback)
    {
        m_change_callback = callback;
    }

    const std::vector<Button*>& Button::button_group() const
    {
        return m_button_group;
    }

    void Button::set_button_group(const std::vector<Button*>& button_group)
    {
        m_button_group = button_group;
    }

    ds::dims<f32> Button::preferred_size() const
    {
        auto&& context{ m_renderer->context() };
        const f32 font_size{ m_font_size == -1.0f ? m_theme->button_font_size : m_font_size };

        nvg::font_size(context, font_size);
        nvg::font_face(context, Font::Name::Sans);

        ds::dims icon_size{ 0.0f, font_size };
        const f32 text_width{ nvg::text_bounds(context, 0.0f, 0.0f, m_text.c_str(), nullptr,
                                               nullptr) };

        if (m_icon != Icon::None)
        {
            if (Icon::is_font(m_icon))
            {
                icon_size.height *= this->icon_scale();
                nvg::font_face(context, Font::Name::Icons);
                nvg::font_size(context, icon_size.height);
                icon_size.width = nvg::text_bounds(context, 0.0f, 0.0f, utf8(m_icon).data(),
                                                   nullptr, nullptr) +
                                  m_size.height * 0.15f;
            }
            else
            {
                icon_size.height *= 0.9f;
                ds::dims image_size{ 0, 0 };
                nvg::image_size(context, std::to_underlying(m_icon), &image_size.width,
                                &image_size.height);

                icon_size.width = (image_size.width * icon_size.height / image_size.height);
            }
        }

        return ds::dims{
            text_width + icon_size.width + 20.0f,
            font_size + 10.0f,
        };
    }

    bool Button::on_mouse_entered(const Mouse& mouse)
    {
        return Widget::on_mouse_entered(mouse);
    }

    bool Button::on_mouse_exited(const Mouse& mouse)
    {
        return Widget::on_mouse_exited(mouse);
    }

    bool Button::handle_mouse_button_event(
        const ds::point<f32>& pt, const Mouse::Button::ID button,
                                           const bool button_just_pressed,
                                           Keyboard::Scancode::ID keys_down)
    {

        // Temporarily increase the reference count of the button in
        // case the button causes the parent window to be destructed
        ds::shared self{ this };

        const bool lmb_and_menu_btn{ 
            button == Mouse::Button::Left && 
            !this->has_property(Property::StandardMenu) };

        const bool rmb_and_not_menu_btn{ 
            button == Mouse::Button::Right && 
            this->has_property(Property::StandardMenu) };

        if (m_enabled && (lmb_and_menu_btn || rmb_and_not_menu_btn))
        {
            const bool pushed_backup{ m_pressed };
            if (button_just_pressed)
            {
                if (this->has_property(Property::Radio))
                {
                    if (m_button_group.empty())
                        for (const auto widget : parent()->children())
                        {
                            const auto btn{ dynamic_cast<Button*>(widget) };
                            if (btn != this && btn != nullptr &&
                                btn->has_property(Property::Radio) && btn->pressed())
                            {
                                btn->m_pressed = false;
                                if (btn->m_change_callback != nullptr)
                                    btn->m_change_callback(false);
                            }
                        }
                    else
                    {
                        for (const auto btn : m_button_group)
                            if (btn != this && btn->has_property(Property::Radio) &&
                                btn->m_pressed)
                            {
                                btn->m_pressed = false;
                                if (btn->m_change_callback != nullptr)
                                    btn->m_change_callback(false);
                            }
                    }
                }

                if (this->has_property(Property::PopupMenu))
                {
                    for (const auto widget : this->parent()->children())
                    {
                        const auto btn{ dynamic_cast<Button*>(widget) };
                        if (btn != this && btn != nullptr &&
                            btn->has_property(Property::PopupMenu) && btn->pressed())
                        {
                            btn->set_pressed(false);
                            if (btn->m_change_callback != nullptr)
                                btn->m_change_callback(false);
                        }
                    }

                    dynamic_cast<PopupButton*>(this)->popup()->request_focus();
                }

                m_pressed = this->has_property(Property::Toggle) ? !m_pressed : true;
            }
            else if (m_pressed || this->has_property(Property::StandardMenu))
            {
                if (m_callback != nullptr && this->contains(pt))
                    m_callback();
                if (this->has_property(Property::StandardPush))
                    m_pressed = false;
            }

            if (pushed_backup != m_pressed && m_change_callback != nullptr)
                m_change_callback(m_pressed);

            return true;
        }
        return false;
    }

    bool Button::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_button_pressed(mouse, kb);
        return handle_mouse_button_event(mouse.pos(), mouse.button_pressed(), true, kb.keys_down());
    }

    bool Button::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_button_released(mouse, kb);
        return handle_mouse_button_event(mouse.pos(), mouse.button_released(), false,
                                         kb.keys_down());
    }

    void Button::draw()
    {
        Widget::draw();

        nvg::NVGcolor grad_top{ m_theme->button_gradient_top_unfocused.nvg() };
        nvg::NVGcolor grad_bot{ m_theme->button_gradient_bot_unfocused.nvg() };

        auto&& context{ m_renderer->context() };
        if (m_pressed || (m_mouse_focus && this->has_property(Property::StandardMenu)))
        {
            grad_top = m_theme->button_gradient_top_pushed.nvg();
            grad_bot = m_theme->button_gradient_bot_pushed.nvg();
        }
        else if (m_mouse_focus && m_enabled)
        {
            grad_top = m_theme->button_gradient_top_focused.nvg();
            grad_bot = m_theme->button_gradient_bot_focused.nvg();
        }

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_pos.x + 1.0f, m_pos.y + 1.0f, m_size.width - 2.0f,
                          m_size.height - 2.0f, m_theme->button_corner_radius - 1.0f);

        if (m_background_color.a != 0)
        {
            nvg::fill_color(context, m_background_color.nvg());
            nvg::fill(context);

            if (m_pressed)
                grad_top.a = grad_bot.a = 0.8f;
            else
            {
                const f32 v{ 1.0f - m_background_color.a };
                grad_top.a = m_enabled ? v : v * 0.5f + 0.5f;
                grad_bot.a = m_enabled ? v : v * 0.5f + 0.5f;
            }
        }

        const nvg::NVGpaint bg{ nvg::linear_gradient(context, m_pos.x, m_pos.y, m_pos.x,
                                                     m_pos.y + m_size.height, grad_top, grad_bot) };
        nvg::fill_paint(context, bg);
        nvg::fill(context);

        nvg::begin_path(context);
        nvg::stroke_width(context, 1.0f);
        nvg::rounded_rect(context, m_pos.x + 0.5f, m_pos.y + (m_pressed ? 0.5f : 1.5f),
                          m_size.width - 1.0f, m_size.height - 1.0f - (m_pressed ? 0.0f : 1.0f),
                          m_theme->button_corner_radius);
        nvg::stroke_color(context, m_theme->border_light.nvg());
        nvg::stroke(context);

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_pos.x + 0.5f, m_pos.y + 0.5f, m_size.width - 1.0f,
                          m_size.height - 2.0f, m_theme->button_corner_radius);
        nvg::stroke_color(context, m_theme->border_dark.nvg());
        nvg::stroke(context);

        f32 font_size{ m_font_size == -1 ? m_theme->button_font_size : m_font_size };
        nvg::font_size(context, font_size);
        nvg::font_face(context, Font::Name::Sans);
        f32 text_width{ nvg::text_bounds(context, 0.0f, 0.0f, m_text.c_str(), nullptr, nullptr) };

        ds::point center{
            m_pos.x + m_size.width * 0.5f,
            m_pos.y + m_size.height * 0.5f,
        };

        ds::point text_pos{
            center.x - text_width * 0.5f,
            center.y - 1.0f,
        };

        nvg::NVGcolor text_color{ m_text_color.a == 0.0f ? m_theme->text_color.nvg()
                                                         : m_text_color.nvg() };
        if (!m_enabled)
            text_color = m_theme->disabled_text_color.nvg();

        if (m_icon != Icon::None)
        {
            const std::string icon{ utf8(std::to_underlying(m_icon)) };
            ds::dims icon_size{ font_size, font_size };

            if (Icon::is_font(m_icon))
            {
                icon_size.height *= this->icon_scale();
                nvg::font_size(context, icon_size.height);
                nvg::font_face(context, Font::Name::Icons);
                icon_size.width = nvg::text_bounds(context, 0, 0, icon.data(), nullptr, nullptr);
            }
            else
            {
                icon_size.height *= 0.9f;
                ds::dims image_size{ 0, 0 };
                nvg::image_size(context, m_icon, &image_size.width, &image_size.height);
                icon_size.width = image_size.height * icon_size.height / image_size.height;
            }

            if (!m_text.empty())
                icon_size.width += m_size.height * 0.15f;

            nvg::fill_color(context, text_color);
            nvg::text_align(context, Text::Alignment::HLeftVMiddle);
            ds::point icon_pos{ center };

            icon_pos.y -= 1;
            switch (m_icon_placement)
            {
                case Icon::Placement::LeftCentered:
                    icon_pos.x -= (text_width + icon_size.width) * 0.5f;
                    text_pos.x += icon_size.width * 0.5f;
                    break;
                case Icon::Placement::RightCentered:
                    text_pos.x -= icon_size.width * 0.5f;
                    icon_pos.x += text_width * 0.5f;
                    break;
                case Icon::Placement::Left:
                    icon_pos.x = m_pos.x + 8.0f;
                    break;
                case Icon::Placement::Right:
                    icon_pos.x = m_pos.x + m_size.width - icon_size.width - 8.0f;
                    break;
            }

            if (Icon::is_font(m_icon))
                nvg::text(context, icon_pos.x, icon_pos.y + 1.0f, icon.data(), nullptr);
            else
            {
                const nvg::NVGpaint img_paint{
                    nvg::image_pattern(context, icon_pos.x, icon_pos.y - (icon_size.height / 2.0f),
                                       icon_size.width, icon_size.height, 0.0f, m_icon,
                                       m_enabled ? 0.5f : 0.25f),
                };
                nvg::fill_paint(context, img_paint);
                nvg::fill(context);
            }
        }

        nvg::font_size(context, font_size);
        nvg::font_face(context, Font::Name::Mono);
        nvg::text_align(context, Text::HLeftVMiddle);
        nvg::fill_color(context, m_theme->text_shadow_color.nvg());
        nvg::text(context, text_pos.x, text_pos.y, m_text.c_str(), nullptr);
        nvg::fill_color(context, text_color);
        nvg::text(context, text_pos.x, text_pos.y + 1.0f, m_text.c_str(), nullptr);
    }
}
