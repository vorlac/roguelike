#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/button.hpp"
#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    Button::Button(Widget* parent, const std::string& caption, Icon::ID icon)
        : Widget{ parent }
        , m_caption{ caption }
        , m_icon{ icon }
        , m_icon_placement{ Icon::Placement::LeftCentered }
        , m_pressed{ false }
        , m_props{ Property::StandardPush }
        , m_background_color{ m_theme ? m_theme->button_gradient_top_focused : Colors::Grey }
        , m_text_color{ m_theme ? m_theme->text_color : Colors::White }
    {
    }

    bool Button::has_property(Button::Property prop) const
    {
        return (std::to_underlying(m_props) & std::to_underlying(prop)) != 0;
    }

    void Button::set_property(Button::Property prop)
    {
        m_props = prop;
    }

    Button::Property Button::properties() const
    {
        return m_props;
    }

    const std::string& Button::caption() const
    {
        return m_caption;
    }

    void Button::set_caption(const std::string& caption)
    {
        m_caption = caption;
    }

    ds::color<f32> Button::background_color() const
    {
        return m_background_color;
    }

    void Button::set_background_color(ds::color<f32> bg_color)
    {
        m_background_color = bg_color;
    }

    ds::color<f32> Button::text_color() const
    {
        return m_text_color;
    }

    void Button::set_text_color(ds::color<f32> text_color)
    {
        m_text_color = text_color;
    }

    Icon::ID Button::icon() const
    {
        return m_icon;
    }

    void Button::set_icon(Icon::ID icon)
    {
        m_icon = icon;
    }

    Icon::Placement Button::icon_placement() const
    {
        return m_icon_placement;
    }

    void Button::set_icon_placement(Icon::Placement placement)
    {
        m_icon_placement = placement;
    }

    bool Button::pressed() const
    {
        return m_pressed;
    }

    void Button::set_pressed(bool pressed)
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

        nvg::FontSize(context, font_size);
        nvg::FontFace(context, font::name::sans);

        ds::dims<f32> icon_size{ 0.0f, font_size };
        const f32 text_width{ nvg::TextBounds(context, 0.0f, 0.0f, m_caption.c_str(), nullptr,
                                              nullptr) };

        if (m_icon != Icon::None)
        {
            if (Icon::is_font(m_icon))
            {
                icon_size.height *= icon_scale();
                nvg::FontFace(context, font::name::icons);
                nvg::FontSize(context, icon_size.height);
                icon_size.width = nvg::TextBounds(context, 0.0f, 0.0f, utf8(m_icon).data(), nullptr,
                                                  nullptr) +
                                  m_size.height * 0.15f;
            }
            else
            {
                icon_size.height *= 0.9f;
                ds::dims<i32> image_size{ 0, 0 };
                nvg::ImageSize(context, std::to_underlying(m_icon), &image_size.width,
                               &image_size.height);

                icon_size.width = image_size.width * icon_size.height / image_size.height;
            }
        }

        return ds::dims<f32>{
            (text_width + icon_size.width) + 20.0f,
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

    bool Button::handle_mouse_button_event(const ds::point<i32>& pt, Mouse::Button::ID mouse_btn,
                                           bool button_just_pressed,
                                           Keyboard::Scancode::ID keys_down)
    {
        const bool isLMBandMenuButton{ mouse_btn == Mouse::Button::Left &&
                                       !this->has_property(Property::StandardMenu) };
        const bool isRMBandNotMenuBtn{ mouse_btn == Mouse::Button::Right &&
                                       this->has_property(Property::StandardMenu) };

        // Temporarily increase the reference count of the button in
        // case the button causes the parent window to be destructed
        ds::shared<Button> self{ this };
        if (m_enabled && (isLMBandMenuButton || isRMBandNotMenuBtn))
        {
            const bool pushed_backup{ m_pressed };
            if (button_just_pressed)
            {
                if (this->has_property(Property::Radio))
                {
                    if (m_button_group.empty())
                    {
                        for (auto& widget : parent()->children())
                        {
                            Button* button{ dynamic_cast<Button*>(widget) };
                            if (button != this && button != nullptr &&
                                button->has_property(Property::Radio) && button->m_pressed)
                            {
                                button->m_pressed = false;
                                if (button->m_change_callback != nullptr)
                                    button->m_change_callback(false);
                            }
                        }
                    }
                    else
                    {
                        for (auto& button : m_button_group)
                        {
                            if (button != this && button->has_property(Property::Radio) &&
                                button->m_pressed)
                            {
                                button->m_pressed = false;
                                if (button->m_change_callback != nullptr)
                                    button->m_change_callback(false);
                            }
                        }
                    }
                }

                if (this->has_property(Property::PopupMenu))
                {
                    for (auto widget : this->parent()->children())
                    {
                        Button* button{ dynamic_cast<Button*>(widget) };
                        if (button != this && button != nullptr &&
                            button->has_property(Property::PopupMenu) && button->m_pressed)
                        {
                            button->set_pressed(false);
                            if (button->m_change_callback != nullptr)
                                button->m_change_callback(false);
                        }
                    }

                    dynamic_cast<PopupButton*>(this)->popup()->request_focus();
                }

                if (this->has_property(Property::Toggle))
                    m_pressed = !m_pressed;
                else
                    m_pressed = true;
            }
            else if (m_pressed || this->has_property(Property::StandardMenu))
            {
                if (this->contains(pt) && m_callback != nullptr)
                    m_callback();
                if (this->has_property(Property::StandardPush))
                    m_pressed = false;
            }

            if (pushed_backup != m_pressed && m_change_callback)
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

        nvg::NVGcolor grad_top{ std::forward<const nvg::NVGcolor>(
            m_theme->button_gradient_top_unfocused) };
        nvg::NVGcolor grad_bot{ std::forward<const nvg::NVGcolor>(
            m_theme->button_gradient_bot_unfocused) };

        auto&& context{ m_renderer->context() };
        if (m_pressed || (m_mouse_focus && this->has_property(Property::StandardMenu)))
        {
            grad_top = m_theme->button_gradient_top_pushed;
            grad_bot = m_theme->button_gradient_bot_pushed;
        }
        else if (m_mouse_focus && m_enabled)
        {
            grad_top = m_theme->button_gradient_top_focused;
            grad_bot = m_theme->button_gradient_bot_focused;
        }

        nvg::BeginPath(context);
        nvg::RoundedRect(context, m_pos.x + 1.0f, m_pos.y + 1.0f, m_size.width - 2.0f,
                         m_size.height - 2.0f, m_theme->button_corner_radius - 1.0f);

        if (m_background_color.a != 0)
        {
            nvg::FillColor(context, m_background_color);
            nvg::Fill(context);

            if (m_pressed)
                grad_top.a = grad_bot.a = 0.8f;
            else
            {
                const f32 v{ 1.0f - m_background_color.a };
                grad_top.a = m_enabled ? v : v * 0.5f + 0.5f;
                grad_bot.a = m_enabled ? v : v * 0.5f + 0.5f;
            }
        }

        const nvg::NVGpaint bg{ nvg::LinearGradient(context, m_pos.x, m_pos.y, m_pos.x,
                                                    m_pos.y + m_size.height, grad_top, grad_bot) };
        nvg::FillPaint(context, bg);
        nvg::Fill(context);

        nvg::BeginPath(context);
        nvg::StrokeWidth(context, 1.0f);
        nvg::RoundedRect(context, m_pos.x + 0.5f, m_pos.y + (m_pressed ? 0.5f : 1.5f),
                         m_size.width - 1.0f, m_size.height - 1.0f - (m_pressed ? 0.0f : 1.0f),
                         m_theme->button_corner_radius);
        nvg::StrokeColor(context, m_theme->border_light);
        nvg::Stroke(context);

        nvg::BeginPath(context);
        nvg::RoundedRect(context, m_pos.x + 0.5f, m_pos.y + 0.5f, m_size.width - 1.0f,
                         m_size.height - 2.0f, m_theme->button_corner_radius);
        nvg::StrokeColor(context, m_theme->border_dark);
        nvg::Stroke(context);

        f32 font_size{ m_font_size == -1 ? m_theme->button_font_size : m_font_size };
        nvg::FontSize(context, font_size);
        nvg::FontFace(context, font::name::sans);
        f32 text_width{ nvg::TextBounds(context, 0.0f, 0.0f, m_caption.c_str(), nullptr, nullptr) };

        ds::point<f32> center{
            m_pos.x + m_size.width * 0.5f,
            m_pos.y + m_size.height * 0.5f,
        };

        ds::point<f32> text_pos{
            center.x - text_width * 0.5f,
            center.y - 1.0f,
        };

        nvg::NVGcolor text_color{ std::forward<const nvg::NVGcolor>(
            m_text_color.a == 0.0f ? m_theme->text_color : m_text_color) };

        if (!m_enabled)
            text_color = m_theme->disabled_text_color;

        if (m_icon != Icon::None)
        {
            const std::string icon{ utf8(std::to_underlying(m_icon)) };
            ds::dims<f32> icon_size{ font_size, font_size };

            if (Icon::is_font(m_icon))
            {
                icon_size.height *= this->icon_scale();
                nvg::FontSize(context, icon_size.height);
                nvg::FontFace(context, font::name::icons);
                icon_size.width = nvg::TextBounds(context, 0, 0, icon.data(), nullptr, nullptr);
            }
            else
            {
                icon_size.height *= 0.9f;
                ds::dims<i32> image_size{ 0, 0 };
                nvg::ImageSize(context, m_icon, &image_size.width, &image_size.height);
                icon_size.width = image_size.height * icon_size.height / image_size.height;
            }

            if (!m_caption.empty())
                icon_size.width += m_size.height * 0.15f;

            nvg::FillColor(context, text_color);
            nvg::TextAlign(context, Text::Alignment::HLeftVMiddle);
            ds::point<f32> icon_pos{ center };

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
                nvg::Text(context, icon_pos.x, icon_pos.y + 1.0f, icon.data(), nullptr);
            else
            {
                const nvg::NVGpaint img_paint{
                    nvg::ImagePattern(context, icon_pos.x, icon_pos.y - (icon_size.height / 2.0f),
                                      icon_size.width, icon_size.height, 0.0f, m_icon,
                                      m_enabled ? 0.5f : 0.25f),
                };
                nvg::FillPaint(context, img_paint);
                nvg::Fill(context);
            }
        }

        nvg::FontSize(context, font_size);
        nvg::FontFace(context, font::name::mono);
        nvg::TextAlign(context, Text::HLeftVMiddle);
        nvg::FillColor(context, m_theme->text_shadow_color);
        nvg::Text(context, text_pos.x, text_pos.y, m_caption.c_str(), nullptr);
        nvg::FillColor(context, text_color);
        nvg::Text(context, text_pos.x, text_pos.y + 1.0f, m_caption.c_str(), nullptr);
    }

}
