#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/button.hpp"
#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    Button::Button(ui::Widget* parent, const std::string& caption, ui::Icon::ID icon)
        : ui::Widget{ parent }
        , m_caption{ caption }
        , m_icon{ icon }
        , m_icon_position{ Icon::Position::LeftCentered }
        , m_pressed{ false }
        , m_flags{ Button::Flags::NormalButton }
        , m_background_color{ m_theme ? m_theme->m_button_gradient_top_focused : Colors::Grey }
        , m_text_color{ m_theme ? m_theme->m_text_color : Colors::White }
    {
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

    void Button::set_background_color(ds::color<f32> background_color)
    {
        m_background_color = background_color;
    }

    ds::color<f32> Button::text_color() const
    {
        return m_text_color;
    }

    void Button::set_text_color(ds::color<f32> text_color)
    {
        m_text_color = text_color;
    }

    ui::Icon::ID Button::icon() const
    {
        return m_icon;
    }

    void Button::set_icon(ui::Icon::ID icon)
    {
        m_icon = icon;
    }

    ui::Button::Flags Button::flags() const
    {
        return m_flags;
    }

    void Button::set_flags(ui::Button::Flags button_flags)
    {
        m_flags = button_flags;
    }

    ui::Icon::Position Button::icon_position() const
    {
        return m_icon_position;
    }

    void Button::set_icon_position(ui::Icon::Position icon_position)
    {
        m_icon_position = icon_position;
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

    ds::dims<f32> Button::preferred_size(nvg::NVGcontext* ctx) const
    {
        f32 font_size{ m_font_size == -1.0f ? m_theme->m_button_font_size : m_font_size };

        nvg::FontSize(ctx, font_size);
        nvg::FontFace(ctx, font::name::sans_bold);

        f32 tw{ nvg::TextBounds(ctx, 0.0f, 0.0f, m_caption.c_str(), nullptr, nullptr) };
        f32 iw{ 0.0f };
        f32 ih{ font_size };

        if (m_icon != ui::Icon::None)
        {
            if (Icon::is_font(m_icon))
            {
                ih *= icon_scale();
                nvg::FontFace(ctx, "icons");
                nvg::FontSize(ctx, ih);
                iw = nvg::TextBounds(ctx, 0.0f, 0.0f, utf8(std::to_underlying(m_icon)).data(),
                                     nullptr, nullptr) +
                     m_size.height * 0.15f;
            }
            else
            {
                i32 w{ 0 };
                i32 h{ 0 };

                ih *= 0.9f;
                nvg::ImageSize(ctx, std::to_underlying(m_icon), &w, &h);
                iw = w * ih / h;
            }
        }

        return ds::dims<f32>{
            tw + iw + 20.0f,
            font_size + 10.0f,
        };
    }

    bool Button::on_mouse_entered(const Mouse& mouse)
    {
        return ui::Widget::on_mouse_entered(mouse);
    }

    bool Button::on_mouse_exited(const Mouse& mouse)
    {
        return ui::Widget::on_mouse_exited(mouse);
    }

    bool Button::handle_mouse_button_event(const ds::point<i32>& pt, Mouse::Button::ID button,
                                           bool button_just_pressed,
                                           Keyboard::Scancode::ID keys_down)
    {
        // Temporarily increase the reference count of the button in
        // case the button causes the parent window to be destructed
        ds::shared<Button> self{ this };

        const bool isLMBandMenuButton{ button == Mouse::Button::Left &&
                                       (m_flags & MenuButton) == 0 };
        const bool isRMBandNotMenuBtn{ button == Mouse::Button::Right &&
                                       (m_flags & MenuButton) != 0 };

        if (m_enabled && (isLMBandMenuButton || isRMBandNotMenuBtn))
        {
            bool pushed_backup{ m_pressed };
            if (button_just_pressed)
            {
                if (m_flags & Button::Flags::RadioButton)
                {
                    if (m_button_group.empty())
                    {
                        for (auto widget : parent()->children())
                        {
                            Button* b = dynamic_cast<Button*>(widget);
                            if (b != this && b && (b->flags() & RadioButton) && b->m_pressed)
                            {
                                b->m_pressed = false;
                                if (b->m_change_callback)
                                    b->m_change_callback(false);
                            }
                        }
                    }
                    else
                    {
                        for (auto b : m_button_group)
                        {
                            if (b != this && (b->flags() & RadioButton) && b->m_pressed)
                            {
                                b->m_pressed = false;
                                if (b->m_change_callback)
                                    b->m_change_callback(false);
                            }
                        }
                    }
                }

                if (m_flags & PopupButton)
                {
                    for (auto widget : this->parent()->children())
                    {
                        Button* b = dynamic_cast<Button*>(widget);
                        if (b != this && b && (b->flags() & PopupButton) && b->m_pressed)
                        {
                            b->m_pressed = false;
                            if (b->m_change_callback)
                                b->m_change_callback(false);
                        }
                    }

                    dynamic_cast<ui::PopupButton*>(this)->popup()->request_focus();
                }

                if (m_flags & ToggleButton)
                    m_pressed = !m_pressed;
                else
                    m_pressed = true;
            }
            else if (m_pressed || (m_flags & MenuButton))
            {
                if (this->contains(pt) && m_callback != nullptr)
                    m_callback();
                if (m_flags & NormalButton)
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
        ui::Widget::on_mouse_button_pressed(mouse, kb);
        return handle_mouse_button_event(mouse.pos(), mouse.button_pressed(), true, kb.keys_down());
    }

    bool Button::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        ui::Widget::on_mouse_button_released(mouse, kb);
        return handle_mouse_button_event(mouse.pos(), mouse.button_released(), false,
                                         kb.keys_down());
    }

    void Button::draw(nvg::NVGcontext* ctx)
    {
        ui::Widget::draw(ctx);

        nvg::NVGcolor grad_top = m_theme->m_button_gradient_top_unfocused;
        nvg::NVGcolor grad_bot = m_theme->m_button_gradient_bot_unfocused;

        if (m_pressed || (m_mouse_focus && (m_flags & MenuButton)))
        {
            grad_top = m_theme->m_button_gradient_top_pushed;
            grad_bot = m_theme->m_button_gradient_bot_pushed;
        }
        else if (m_mouse_focus && m_enabled)
        {
            grad_top = m_theme->m_button_gradient_top_focused;
            grad_bot = m_theme->m_button_gradient_bot_focused;
        }

        nvg::BeginPath(ctx);
        nvg::RoundedRect(ctx, m_pos.x + 1.0f, m_pos.y + 1.0f, m_size.width - 2.0f,
                         m_size.height - 2.0f, m_theme->m_button_corner_radius - 1.0f);

        if (m_background_color.a != 0)
        {
            nvg::FillColor(ctx, m_background_color);
            nvg::Fill(ctx);

            if (m_pressed)
                grad_top.a = grad_bot.a = 0.8f;
            else
            {
                f32 v{ 1.0f - m_background_color.a };
                grad_top.a = m_enabled ? v : v * 0.5f + 0.5f;
                grad_bot.a = m_enabled ? v : v * 0.5f + 0.5f;
            }
        }

        nvg::NVGpaint bg{ nvg::LinearGradient(ctx, m_pos.x, m_pos.y, m_pos.x,
                                              m_pos.y + m_size.height, grad_top, grad_bot) };
        nvg::FillPaint(ctx, bg);
        nvg::Fill(ctx);

        nvg::BeginPath(ctx);
        nvg::StrokeWidth(ctx, 1.0f);
        nvg::RoundedRect(ctx, m_pos.x + 0.5f, m_pos.y + (m_pressed ? 0.5f : 1.5f),
                         m_size.width - 1.0f, m_size.height - 1.0f - (m_pressed ? 0.0f : 1.0f),
                         m_theme->m_button_corner_radius);
        nvg::StrokeColor(ctx, m_theme->m_border_light);
        nvg::Stroke(ctx);

        nvg::BeginPath(ctx);
        nvg::RoundedRect(ctx, m_pos.x + 0.5f, m_pos.y + 0.5f, m_size.width - 1.0f,
                         m_size.height - 2.0f, m_theme->m_button_corner_radius);
        nvg::StrokeColor(ctx, m_theme->m_border_dark);
        nvg::Stroke(ctx);

        f32 font_size{ m_font_size == -1 ? m_theme->m_button_font_size : m_font_size };
        nvg::FontSize(ctx, font_size);
        nvg::FontFace(ctx, "sans-bold");
        f32 tw{ nvg::TextBounds(ctx, 0.0f, 0.0f, m_caption.c_str(), nullptr, nullptr) };

        ds::point<f32> center{
            m_pos.x + m_size.width * 0.5f,
            m_pos.y + m_size.height * 0.5f,
        };

        ds::point<f32> text_pos{
            center.x - tw * 0.5f,
            center.y - 1.0f,
        };

        nvg::NVGcolor text_color{ m_text_color.a == 0.0f
                                      ? m_theme->m_text_color
                                      : static_cast<nvg::NVGcolor>(m_text_color) };

        if (!m_enabled)
            text_color = m_theme->m_disabled_text_color;

        if (m_icon != ui::Icon::None)
        {
            std::string icon{ utf8(std::to_underlying(m_icon)) };
            f32 iw{ font_size };
            f32 ih{ font_size };

            if (Icon::is_font(m_icon))
            {
                ih *= this->icon_scale();
                nvg::FontSize(ctx, ih);
                nvg::FontFace(ctx, "icons");
                iw = nvg::TextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
            }
            else
            {
                i32 w{ 0 };
                i32 h{ 0 };

                ih *= 0.9f;
                nvg::ImageSize(ctx, std::to_underlying(m_icon), &w, &h);
                iw = w * ih / h;
            }

            if (!m_caption.empty())
                iw += m_size.height * 0.15f;

            nvg::FillColor(ctx, text_color);
            nvg::TextAlign(ctx, nvg::NVG_ALIGN_LEFT | nvg::NVG_ALIGN_MIDDLE);
            ds::point<f32> icon_pos = center;
            icon_pos.y -= 1;

            switch (m_icon_position)
            {
                case Icon::Position::LeftCentered:
                {
                    icon_pos.x -= (tw + iw) * 0.5f;
                    text_pos.x += iw * 0.5f;
                    break;
                }
                case Icon::Position::RightCentered:
                {
                    text_pos.x -= iw * 0.5f;
                    icon_pos.x += tw * 0.5f;
                    break;
                }
                case Icon::Position::Left:
                {
                    icon_pos.x = m_pos.x + 8.0f;
                    break;
                }
                case Icon::Position::Right:
                {
                    icon_pos.x = m_pos.x + m_size.width - iw - 8.0f;
                    break;
                }
            }

            if (Icon::is_font(m_icon))
                nvg::Text(ctx, icon_pos.x, icon_pos.y + 1.0f, icon.data(), nullptr);
            else
            {
                nvg::NVGpaint img_paint{ nvg::ImagePattern(ctx, icon_pos.x, icon_pos.y - ih / 2.0f,
                                                           iw, ih, 0.0f, m_icon,
                                                           m_enabled ? 0.5f : 0.25f) };

                nvg::FillPaint(ctx, img_paint);
                nvg::Fill(ctx);
            }
        }

        nvg::FontSize(ctx, font_size);
        nvg::FontFace(ctx, "sans-bold");
        nvg::TextAlign(ctx, nvg::NVG_ALIGN_LEFT | nvg::NVG_ALIGN_MIDDLE);
        nvg::FillColor(ctx, m_theme->m_text_shadow_color);
        nvg::Text(ctx, text_pos.x, text_pos.y, m_caption.c_str(), nullptr);
        nvg::FillColor(ctx, text_color);
        nvg::Text(ctx, text_pos.x, text_pos.y + 1, m_caption.c_str(), nullptr);
    }

}
