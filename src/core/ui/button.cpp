#include <nanovg.h>

#include "core/mouse.hpp"
#include "core/ui/button.hpp"
#include "core/ui/popupbutton.hpp"
#include "core/ui/theme.hpp"
#include "utils/unicode.hpp"

namespace rl::ui {

    /// @brief
    ///     Determine whether an icon ID is a texture loaded via nvg_image_icon.
    ///
    ///     The implementation defines all value { 1024 as image icons, and
    ///     everything }= 1024 as an Entypo icon (see ). The value 1024 exists to
    ///     provide a generous buffer on how many images may have been loaded by
    ///     NanoVG.
    static inline bool nvg_is_image_icon(int value)
    {
        return value < 1024;
    }

    /// @brief
    ///     Determine whether an icon ID is a font-based icon (e.g. from
    ///     font.ttf).
    static inline bool nvg_is_font_icon(int value)
    {
        return value >= 1024;
    }

    Button::Button(ui::widget* parent, const std::string& caption, i32 icon)
        : ui::widget{ parent }
        , m_caption{ caption }
        , m_icon{ icon }
        , m_icon_position{ IconPosition::LeftCentered }
        , m_pressed{ false }
        , m_flags{ Flags::NormalButton }
        , m_background_color{ 0, 0, 0 }
        , m_text_color{ 0, 0, 0 }
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

    const ds::color<u8>& Button::background_color() const
    {
        return m_background_color;
    }

    void Button::set_background_color(const ds::color<u8>& background_color)
    {
        m_background_color = background_color;
    }

    const ds::color<u8>& Button::text_color() const
    {
        return m_text_color;
    }

    void Button::set_text_color(const ds::color<u8>& text_color)
    {
        m_text_color = text_color;
    }

    i32 Button::icon() const
    {
        return m_icon;
    }

    void Button::set_icon(i32 icon)
    {
        m_icon = icon;
    }

    i32 Button::flags() const
    {
        return m_flags;
    }

    void Button::set_flags(Button::Flags button_flags)
    {
        m_flags = button_flags;
    }

    Button::IconPosition Button::icon_position() const
    {
        return m_icon_position;
    }

    void Button::set_icon_position(Button::IconPosition icon_position)
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

    ds::dims<i32> Button::preferred_size(NVGcontext* ctx) const
    {
        i32 font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;

        nvgFontSize(ctx, font_size);
        nvgFontFace(ctx, "sans-bold");

        f32 tw{ nvgTextBounds(ctx, 0, 0, m_caption.c_str(), nullptr, nullptr) };
        f32 iw{ 0.0f };
        f32 ih{ static_cast<f32>(font_size) };

        if (m_icon != 0)
        {
            if (nvg_is_font_icon(m_icon))
            {
                ih *= icon_scale();
                nvgFontFace(ctx, "icons");
                nvgFontSize(ctx, ih);
                iw = nvgTextBounds(ctx, 0, 0, utf8(m_icon).data(), nullptr, nullptr) +
                     m_size.height * 0.15f;
            }
            else
            {
                i32 w{ 0 };
                i32 h{ 0 };
                ih *= 0.9f;
                nvgImageSize(ctx, m_icon, &w, &h);
                iw = w * ih / h;
            }
        }
        return ds::dims<i32>(static_cast<int>(tw + iw) + 20, font_size + 10);
    }

    bool Button::on_mouse_entered(ds::point<i32> pos)
    {
        return ui::widget::on_mouse_entered(pos);
    }

    bool Button::on_mouse_exited(ds::point<i32> pos)
    {
        return ui::widget::on_mouse_exited(pos);
    }

    bool Button::handle_mouse_button_event(const ds::point<i32>& pt, rl::Mouse::Button::type button,
                                           bool down, i32 modifiers)
    {
        // Temporarily increase the reference count of the button in case the
        // button causes the parent window to be destructed
        ds::shared<Button> self{ this };

        if (m_enabled == 1 && ((button == rl::Mouse::Button::Left && !(m_flags & MenuButton)) ||
                               (button == rl::Mouse::Button::Right && (m_flags & MenuButton))))
        {
            bool pushed_backup{ m_pressed };
            if (down)
            {
                if (m_flags & RadioButton)
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

    bool Button::on_mouse_button_pressed(ds::point<i32> pos, Mouse::Button::type btn, i32 modifiers)
    {
        ui::widget::on_mouse_button_pressed(pos, btn, modifiers);
        return handle_mouse_button_event(pos, btn, true, modifiers);
    }

    bool Button::on_mouse_button_released(ds::point<i32> pos, Mouse::Button::type btn, i32 modifiers)
    {
        ui::widget::on_mouse_button_released(pos, btn, modifiers);
        return handle_mouse_button_event(pos, btn, false, modifiers);
    }

    void Button::draw(NVGcontext* ctx)
    {
        ui::widget::draw(ctx);

        NVGcolor grad_top = m_theme->m_button_gradient_top_unfocused;
        NVGcolor grad_bot = m_theme->m_button_gradient_bot_unfocused;

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

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x + 1.0f, m_pos.y + 1.0f, m_size.width - 2.0f,
                       m_size.height - 2.0f, m_theme->m_button_corner_radius - 1);

        if (m_background_color.a != 0)
        {
            nvgFillColor(ctx, NVGcolor{
                                  m_background_color.r / 255.0f,
                                  m_background_color.g / 255.0f,
                                  m_background_color.b / 255.0f,
                                  1.0f,
                              });
            nvgFill(ctx);

            if (m_pressed)
                grad_top.a = grad_bot.a = 0.8f;
            else
            {
                f32 v{ 1.0f - m_background_color.a };
                grad_top.a = m_enabled ? v : v * 0.5f + 0.5f;
                grad_bot.a = m_enabled ? v : v * 0.5f + 0.5f;
            }
        }

        NVGpaint bg{ nvgLinearGradient(ctx, m_pos.x, m_pos.y, m_pos.x, m_pos.y + m_size.height,
                                       grad_top, grad_bot) };
        nvgFillPaint(ctx, bg);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgStrokeWidth(ctx, 1.0f);
        nvgRoundedRect(ctx, m_pos.x + 0.5f, m_pos.y + (m_pressed ? 0.5f : 1.5f),
                       m_size.width - 1.0f, m_size.height - 1.0f - (m_pressed ? 0.0f : 1.0f),
                       m_theme->m_button_corner_radius);
        nvgStrokeColor(ctx, m_theme->m_border_light);
        nvgStroke(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x + 0.5f, m_pos.y + 0.5f, m_size.width - 1.0f,
                       m_size.height - 2.0f, m_theme->m_button_corner_radius);
        nvgStrokeColor(ctx, m_theme->m_border_dark);
        nvgStroke(ctx);

        i32 font_size{ m_font_size == -1 ? m_theme->m_button_font_size : m_font_size };
        nvgFontSize(ctx, font_size);
        nvgFontFace(ctx, "sans-bold");
        f32 tw{ nvgTextBounds(ctx, 0, 0, m_caption.c_str(), nullptr, nullptr) };

        ds::point<f32> center{
            m_pos.x + m_size.width * 0.5f,
            m_pos.y + m_size.height * 0.5f,
        };

        ds::point<f32> text_pos{
            center.x - tw * 0.5f,
            center.y - 1,
        };

        NVGcolor text_color = m_text_color.a == 0 ? m_theme->m_text_color : m_text_color;

        if (!m_enabled)
            text_color = m_theme->m_disabled_text_color;

        if (m_icon)
        {
            std::string icon{ utf8(m_icon) };
            f32 iw{ static_cast<f32>(font_size) };
            f32 ih{ static_cast<f32>(font_size) };

            if (nvg_is_font_icon(m_icon))
            {
                ih *= this->icon_scale();
                nvgFontSize(ctx, ih);
                nvgFontFace(ctx, "icons");
                iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
            }
            else
            {
                i32 w{ 0 };
                i32 h{ 0 };
                ih *= 0.9f;
                nvgImageSize(ctx, m_icon, &w, &h);
                iw = w * ih / h;
            }

            if (m_caption != "")
                iw += m_size.height * 0.15f;

            nvgFillColor(ctx, text_color);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            ds::point<f32> icon_pos = center;
            icon_pos.y -= 1;

            if (m_icon_position == IconPosition::LeftCentered)
            {
                icon_pos.x -= (tw + iw) * 0.5f;
                text_pos.x += iw * 0.5f;
            }
            else if (m_icon_position == IconPosition::RightCentered)
            {
                text_pos.x -= iw * 0.5f;
                icon_pos.x += tw * 0.5f;
            }
            else if (m_icon_position == IconPosition::Left)
            {
                icon_pos.x = m_pos.x + 8;
            }
            else if (m_icon_position == IconPosition::Right)
            {
                icon_pos.x = m_pos.x + m_size.width - iw - 8;
            }

            if (nvg_is_font_icon(m_icon))
                nvgText(ctx, icon_pos.x, icon_pos.y + 1, icon.data(), nullptr);
            else
            {
                NVGpaint img_paint = nvgImagePattern(ctx, icon_pos.x, icon_pos.y - ih / 2, iw, ih,
                                                     0, m_icon, m_enabled ? 0.5f : 0.25f);

                nvgFillPaint(ctx, img_paint);
                nvgFill(ctx);
            }
        }

        nvgFontSize(ctx, font_size);
        nvgFontFace(ctx, "sans-bold");
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(ctx, m_theme->m_text_shadow_color);
        nvgText(ctx, text_pos.x, text_pos.y, m_caption.c_str(), nullptr);
        nvgFillColor(ctx, text_color);
        nvgText(ctx, text_pos.x, text_pos.y + 1, m_caption.c_str(), nullptr);
    }

}
