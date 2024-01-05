#include <iostream>
#include <regex>
#include <string>

#include <nanovg.h>

#include "core/ui/textbox.hpp"
#include "core/ui/theme.hpp"
#include "core/window.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"
#include "utils/unicode.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_Clipboard.h>
SDL_C_LIB_END

namespace rl::ui {

    TextBox::TextBox(ui::widget* parent, const std::string& value)
        : ui::widget{ parent }
        , m_editable{ false }
        , m_spinnable{ false }
        , m_committed{ true }
        , m_value{ value }
        , m_default_value{ "" }
        , m_alignment{ Alignment::Center }
        , m_units{ "" }
        , m_format{ "" }
        , m_units_image{ -1 }
        , m_valid_format{ true }
        , m_value_temp{ value }
        , m_cursor_pos{ -1 }
        , m_selection_pos{ -1 }
        , m_mouse_pos{ -1, -1 }
        , m_mouse_down_pos{ -1, -1 }
        , m_mouse_drag_pos{ -1, -1 }
        , m_mouse_down_modifier{ 0 }
        , m_text_offset{ 0 }
        , m_last_click{ 0 }
    {
        if (m_theme)
            m_font_size = m_theme->m_text_box_font_size;

        m_icon_extra_scale = .8f;
    }

    bool TextBox::editable() const
    {
        return m_editable;
    }

    bool TextBox::spinnable() const
    {
        return m_spinnable;
    }

    void TextBox::set_spinnable(bool spinnable)
    {
        m_spinnable = spinnable;
    }

    const std::string& TextBox::value() const
    {
        return m_value;
    }

    void TextBox::set_value(const std::string& value)
    {
        m_value = value;
    }

    const std::string& TextBox::default_value() const
    {
        return m_default_value;
    }

    void TextBox::set_default_value(const std::string& default_value)
    {
        m_default_value = default_value;
    }

    TextBox::Alignment TextBox::alignment() const
    {
        return m_alignment;
    }

    void TextBox::set_alignment(TextBox::Alignment align)
    {
        m_alignment = align;
    }

    const std::string& TextBox::units() const
    {
        return m_units;
    }

    void TextBox::set_units(const std::string& units)
    {
        m_units = units;
    }

    i32 TextBox::units_image() const
    {
        return m_units_image;
    }

    void TextBox::set_units_image(int image)
    {
        m_units_image = image;
    }

    // Return the underlying regular expression specifying valid formats
    const std::string& TextBox::format() const
    {
        return m_format;
    }

    // Specify a regular expression specifying valid formats
    void TextBox::set_format(const std::string& format)
    {
        m_format = format;
    }

    // Return the placeholder text to be displayed while the text box is empty.
    const std::string& TextBox::placeholder() const
    {
        return m_placeholder;
    }

    // Specify a placeholder text to be displayed while the text box is empty.
    void TextBox::set_placeholder(const std::string& placeholder)
    {
        m_placeholder = placeholder;
    }

    // The callback to execute when the value of this TextBox has changed.
    const std::function<bool(const std::string& str)>& TextBox::callback() const
    {
        return m_callback;
    }

    // Sets the callback to execute when the value of this TextBox has changed.
    void TextBox::set_callback(const std::function<bool(const std::string& str)>& callback)
    {
        m_callback = callback;
    }

    void TextBox::set_editable(bool editable)
    {
        m_editable = editable;
        this->set_cursor(editable ? Mouse::Cursor::IBeam : Mouse::Cursor::Arrow);
    }

    void TextBox::set_theme(ui::theme* theme)
    {
        ui::widget::set_theme(theme);
        if (m_theme != nullptr)
            m_font_size = m_theme->m_text_box_font_size;
    }

    ds::dims<i32> TextBox::preferred_size(NVGcontext* ctx) const
    {
        ds::dims<i32> size{
            0,
            static_cast<i32>(this->font_size() * 1.4f),
        };

        f32 uw{ 0.0f };
        if (m_units_image > 0)
        {
            i32 w{ 0 };
            i32 h{ 0 };

            nvgImageSize(ctx, m_units_image, &w, &h);
            f32 uh{ size.height * 0.4f };
            uw = w * uh / h;
        }
        else if (!m_units.empty())
            uw = nvgTextBounds(ctx, 0, 0, m_units.c_str(), nullptr, nullptr);

        f32 sw{ 0.0f };
        if (m_spinnable)
            sw = 14.0f;

        f32 ts{ nvgTextBounds(ctx, 0, 0, m_value.c_str(), nullptr, nullptr) };
        size.width = size.height + ts + uw + sw;

        return size;
    }

    void TextBox::draw(NVGcontext* ctx)
    {
        ui::widget::draw(ctx);

        NVGpaint bg{ nvgBoxGradient(ctx, m_pos.x + 1, m_pos.y + 1 + 1.0f, m_size.width - 2,
                                    m_size.height - 2, 3, 4, ds::color<u8>{ 255, 255, 255, 32 },
                                    ds::color<u8>{ 32, 32, 32, 32 }) };
        NVGpaint fg1{ nvgBoxGradient(ctx, m_pos.x + 1, m_pos.y + 1 + 1.0f, m_size.width - 2,
                                     m_size.height - 2, 3, 4, ds::color<u8>{ 150, 150, 150, 32 },
                                     ds::color<u8>{ 32, 32, 32, 32 }) };
        NVGpaint fg2{ nvgBoxGradient(ctx, m_pos.x + 1, m_pos.y + 1 + 1.0f, m_size.width - 2,
                                     m_size.height - 2, 3, 4, nvgRGBA(255, 0, 0, 100),
                                     nvgRGBA(255, 0, 0, 50)) };

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x + 1, m_pos.y + 1 + 1.0f, m_size.width - 2, m_size.height - 2, 3);

        if (m_editable && this->focused())
            m_valid_format ? nvgFillPaint(ctx, fg1) : nvgFillPaint(ctx, fg2);
        else if (m_spinnable && m_mouse_down_pos.x != -1)
            nvgFillPaint(ctx, fg1);
        else
            nvgFillPaint(ctx, bg);

        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x + 0.5f, m_pos.y + 0.5f, m_size.width - 1, m_size.height - 1,
                       2.5f);
        nvgStrokeColor(ctx, ds::color<u8>{ 0, 0, 0, 48 });
        nvgStroke(ctx);

        nvgFontSize(ctx, this->font_size());
        nvgFontFace(ctx, font::name::sans);

        ds::point<i32> draw_pos{
            m_pos.x,
            static_cast<i32>(m_pos.y + m_size.height * 0.5f + 1),
        };

        f32 x_spacing{ m_size.height * 0.3f };
        f32 unit_width{ 0 };

        if (m_units_image > 0)
        {
            i32 w{ 0 };
            i32 h{ 0 };

            nvgImageSize(ctx, m_units_image, &w, &h);

            f32 unit_height{ m_size.height * 0.4f };
            unit_width = w * unit_height / h;

            NVGpaint img_paint{ nvgImagePattern(
                ctx, m_pos.x + m_size.width - x_spacing - unit_width,
                draw_pos.y - unit_height * 0.5f, unit_width, unit_height, 0, m_units_image,
                m_enabled ? 0.7f : 0.35f) };

            nvgBeginPath(ctx);
            nvgRect(ctx, m_pos.x + m_size.width - x_spacing - unit_width,
                    draw_pos.y - unit_height * 0.5f, unit_width, unit_height);
            nvgFillPaint(ctx, img_paint);
            nvgFill(ctx);

            unit_width += 2;
        }
        else if (!m_units.empty())
        {
            unit_width = nvgTextBounds(ctx, 0, 0, m_units.c_str(), nullptr, nullptr);

            nvgFillColor(ctx, ds::color<u8>{ 255, 255, 255, m_enabled ? 64 : 32 });
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
            nvgText(ctx, m_pos.x + m_size.width - x_spacing, draw_pos.y, m_units.c_str(), nullptr);

            unit_width += 2;
        }

        f32 spin_arrows_width{ 0.0f };
        if (m_spinnable && !focused())
        {
            spin_arrows_width = 14.0f;

            nvgFontFace(ctx, "icons");
            nvgFontSize(ctx, ((m_font_size < 0) ? m_theme->m_button_font_size : m_font_size) *
                                 this->icon_scale());

            bool spinning = m_mouse_down_pos.x != -1;

            {
                // up button
                bool hover{ m_mouse_focus && spin_area(m_mouse_pos) == SpinArea::Top };
                nvgFillColor(ctx, (m_enabled && (hover || spinning))
                                      ? m_theme->m_text_color
                                      : m_theme->m_disabled_text_color);

                auto icon{ utf8(m_theme->m_text_box_up_icon) };
                nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

                ds::point<i32> icon_pos{
                    static_cast<i32>(m_pos.x + 4.0f),
                    static_cast<i32>(m_pos.y + m_size.height / 2.0f - x_spacing / 2.0f),
                };

                nvgText(ctx, icon_pos.x, icon_pos.y, icon.data(), nullptr);
            }

            {
                // down button
                bool hover{ m_mouse_focus && this->spin_area(m_mouse_pos) == SpinArea::Bottom };
                nvgFillColor(ctx, (m_enabled && (hover || spinning))
                                      ? m_theme->m_text_color
                                      : m_theme->m_disabled_text_color);

                auto icon{ utf8(m_theme->m_text_box_down_icon) };
                nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

                ds::point<i32> icon_pos{
                    static_cast<i32>(m_pos.x + 4.0f),
                    static_cast<i32>(m_pos.y + m_size.height / 2.0f + x_spacing / 2.0f + 1.5f),
                };

                nvgText(ctx, icon_pos.x, icon_pos.y, icon.data(), nullptr);
            }

            nvgFontSize(ctx, this->font_size());
            nvgFontFace(ctx, "sans");
        }

        switch (m_alignment)
        {
            case Alignment::Left:
                nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
                draw_pos.x += x_spacing + spin_arrows_width;
                break;
            case Alignment::Right:
                nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
                draw_pos.x += m_size.width - unit_width - x_spacing;
                break;
            case Alignment::Center:
                nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                draw_pos.x += m_size.width * 0.5f;
                break;
        }

        nvgFontSize(ctx, this->font_size());
        nvgFillColor(ctx, m_enabled && (!m_committed || !m_value.empty())
                              ? m_theme->m_text_color
                              : m_theme->m_disabled_text_color);

        // clip visible text area
        f32 clip_x{ m_pos.x + x_spacing + spin_arrows_width - 1.0f };
        f32 clip_y{ m_pos.y + 1.0f };
        f32 clip_width{ m_size.width - unit_width - spin_arrows_width - 2 * x_spacing + 2.0f };
        f32 clip_height{ m_size.height - 3.0f };

        nvgSave(ctx);
        nvgIntersectScissor(ctx, clip_x, clip_y, clip_width, clip_height);

        ds::point<i32> old_draw_pos{ draw_pos };
        draw_pos.x += static_cast<i32>(m_text_offset);

        if (m_committed)
        {
            nvgText(ctx, draw_pos.x, draw_pos.y,
                    m_value.empty() ? m_placeholder.c_str() : m_value.c_str(), nullptr);
        }
        else
        {
            constexpr static i32 max_glyphs{ 1024 };
            NVGglyphPosition glyphs[max_glyphs];

            std::array<f32, 4> text_bound = { 0.0f };
            nvgTextBounds(ctx, static_cast<f32>(draw_pos.x), static_cast<f32>(draw_pos.y),
                          m_value_temp.c_str(), nullptr, text_bound.data());

            f32 lineh{ text_bound[3] - text_bound[1] };

            // find cursor positions
            i32 nglyphs{ nvgTextGlyphPositions(ctx, static_cast<f32>(draw_pos.x),
                                               static_cast<f32>(draw_pos.y), m_value_temp.c_str(),
                                               nullptr, glyphs, max_glyphs) };
            this->update_cursor(ctx, text_bound[2], glyphs, nglyphs);

            // compute text offset
            i32 prev_cpos{ m_cursor_pos > 0 ? m_cursor_pos - 1 : 0 };
            i32 next_cpos{ m_cursor_pos < nglyphs ? m_cursor_pos + 1 : nglyphs };
            f32 prev_cx{ this->cursor_index_to_position(prev_cpos, text_bound[2], glyphs, nglyphs) };
            f32 next_cx{ this->cursor_index_to_position(next_cpos, text_bound[2], glyphs, nglyphs) };

            if (next_cx > clip_x + clip_width)
                m_text_offset -= next_cx - (clip_x + clip_width) + 1;

            if (prev_cx < clip_x)
                m_text_offset += clip_x - prev_cx + 1;

            draw_pos.x = old_draw_pos.x + m_text_offset;

            // draw text with offset
            nvgText(ctx, static_cast<f32>(draw_pos.x), static_cast<f32>(draw_pos.y),
                    m_value_temp.c_str(), nullptr);
            nvgTextBounds(ctx, static_cast<f32>(draw_pos.x), static_cast<f32>(draw_pos.y),
                          m_value_temp.c_str(), nullptr, text_bound.data());

            // recompute cursor positions
            nglyphs = nvgTextGlyphPositions(ctx, draw_pos.x, draw_pos.y, m_value_temp.c_str(),
                                            nullptr, glyphs, max_glyphs);

            if (m_cursor_pos > -1)
            {
                if (m_selection_pos > -1)
                {
                    f32 caretx{ cursor_index_to_position(m_cursor_pos, text_bound[2], glyphs,
                                                         nglyphs) };
                    f32 selx{ cursor_index_to_position(m_selection_pos, text_bound[2], glyphs,
                                                       nglyphs) };
                    if (caretx > selx)
                        std::swap(caretx, selx);

                    // draw selection
                    nvgBeginPath(ctx);
                    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 80));
                    nvgRect(ctx, caretx, draw_pos.y - lineh * 0.5f, selx - caretx, lineh);
                    nvgFill(ctx);
                }

                f32 caretx{ cursor_index_to_position(m_cursor_pos, text_bound[2], glyphs, nglyphs) };

                // draw cursor
                nvgBeginPath(ctx);
                nvgMoveTo(ctx, caretx, draw_pos.y - lineh * 0.5f);
                nvgLineTo(ctx, caretx, draw_pos.y + lineh * 0.5f);
                nvgStrokeColor(ctx, nvgRGBA(255, 192, 0, 255));
                nvgStrokeWidth(ctx, 1.0f);
                nvgStroke(ctx);
            }
        }

        nvgRestore(ctx);
    }

    bool TextBox::on_mouse_entered(const Mouse& mouse)
    {
        ui::widget::on_mouse_entered(mouse);
        return true;
    }

    bool TextBox::on_mouse_exited(const Mouse& mouse)
    {
        ui::widget::on_mouse_exited(mouse);
        return true;
    }

    bool TextBox::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        auto&& mouse_pos{ mouse.pos() };

        if (mouse.is_button_pressed(Mouse::Button::Left) && !m_focused)
        {
            // not on scrolling arrows
            if (!m_spinnable || this->spin_area(mouse_pos) == SpinArea::None)
                this->request_focus();
        }

        if (m_editable && this->focused())
        {
            m_mouse_down_pos = mouse_pos;

            m_mouse_down_modifier = 0;
            if (kb.is_button_down(Keyboard::Button::LCtrl))
                m_mouse_down_modifier |= Keyboard::Button::LCtrl;
            if (kb.is_button_down(Keyboard::Button::LShift))
                m_mouse_down_modifier |= Keyboard::Button::LShift;

            f32 time{ m_timer.elapsed() };
            if (time - m_last_click < 0.25f)
            {
                // Double-click: select all text
                m_selection_pos = 0;
                m_cursor_pos = static_cast<i32>(m_value_temp.size());
                m_mouse_down_pos = { -1, -1 };
            }

            m_last_click = time;
            return true;
        }
        else if (m_spinnable && !this->focused())
        {
            if (this->spin_area(mouse_pos) == SpinArea::None)
            {
                m_mouse_down_pos = mouse_pos;
                m_mouse_down_modifier = 0;
                if (kb.is_button_down(Keyboard::Button::LCtrl))
                    m_mouse_down_modifier |= Keyboard::Button::LCtrl;
                if (kb.is_button_down(Keyboard::Button::LShift))
                    m_mouse_down_modifier |= Keyboard::Button::LShift;

                f32 time{ m_timer.elapsed() };
                if (time - m_last_click < 0.25f)
                {
                    // Double-click: reset to default value
                    m_value = m_default_value;
                    if (m_callback != nullptr)
                        m_callback(m_value);

                    m_mouse_down_pos = { -1, -1 };
                }

                m_last_click = time;
            }
            else
            {
                m_mouse_down_pos = { -1, -1 };
                m_mouse_drag_pos = { -1, -1 };
            }

            return true;
        }

        return false;
    }

    bool TextBox::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if (m_editable && this->focused())
        {
            m_mouse_down_pos = { -1, -1 };
            m_mouse_drag_pos = { -1, -1 };
            return true;
        }
        else if (m_spinnable && !this->focused())
        {
            m_mouse_down_pos = { -1, -1 };
            m_mouse_drag_pos = { -1, -1 };
            return true;
        }

        return false;
    }

    bool TextBox::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        m_mouse_pos = mouse.pos();

        if (!m_editable)
            this->set_cursor(Mouse::Cursor::Arrow);
        // scrolling arrows
        else if (m_spinnable && !this->focused() && this->spin_area(m_mouse_pos) != SpinArea::None)
            this->set_cursor(Mouse::Cursor::Hand);
        else
            this->set_cursor(Mouse::Cursor::IBeam);

        return m_editable;
    }

    bool TextBox::on_mouse_drag(ds::point<i32> pnt, ds::vector2<i32> rel, const Mouse& mouse,
                                const Keyboard& kb)
    {
        m_mouse_pos = mouse.pos();
        m_mouse_drag_pos = mouse.pos();
        return m_editable && this->focused();
    }

    bool TextBox::on_focus_gained()
    {
        ui::widget::on_focus_gained();
        std::string backup{ m_value };

        if (m_editable)
        {
            m_value_temp = m_value;
            m_committed = false;
            m_cursor_pos = 0;
            m_valid_format = m_value_temp.empty() || this->check_format(m_value_temp, m_format);
        }

        return true;
    }

    bool TextBox::on_focus_lost()
    {
        ui::widget::on_focus_lost();
        std::string backup{ m_value };

        if (m_editable)
        {
            if (m_valid_format)
            {
                if (m_value_temp == "")
                    m_value = m_default_value;
                else
                    m_value = m_value_temp;
            }

            if (m_callback && !m_callback(m_value))
                m_value = backup;

            m_committed = true;
            m_cursor_pos = -1;
            m_selection_pos = -1;
            m_text_offset = 0;

            m_valid_format = (m_value_temp == "") || this->check_format(m_value_temp, m_format);
        }

        return true;
    }

    bool TextBox::on_key_pressed(const Keyboard& kb)
    {
        if (m_editable && this->focused())
        {
            // TODO: make sure repeat/hold keys is handled
            if (kb.is_button_pressed(Keyboard::Button::Left))
            {
                if (kb.is_button_down(Keyboard::Button::LShift))
                {
                    if (m_selection_pos == -1)
                        m_selection_pos = m_cursor_pos;
                }
                else
                {
                    m_selection_pos = -1;
                }

                if (m_cursor_pos > 0)
                    m_cursor_pos--;
            }
            else if (kb.is_button_pressed(Keyboard::Button::Right))
            {
                if (kb.is_button_down(Keyboard::Button::Shift))
                {
                    if (m_selection_pos == -1)
                        m_selection_pos = m_cursor_pos;
                }
                else
                {
                    m_selection_pos = -1;
                }

                if (m_cursor_pos < static_cast<int>(m_value_temp.length()))
                    m_cursor_pos++;
            }
            else if (kb.is_button_pressed(Keyboard::Button::Home))
            {
                if (kb.is_button_down(Keyboard::Button::Shift))
                {
                    if (m_selection_pos == -1)
                        m_selection_pos = m_cursor_pos;
                }
                else
                {
                    m_selection_pos = -1;
                }

                m_cursor_pos = 0;
            }
            else if (kb.is_button_pressed(Keyboard::Button::End))
            {
                if (kb.is_button_down(Keyboard::Button::Shift))
                {
                    if (m_selection_pos == -1)
                        m_selection_pos = m_cursor_pos;
                }
                else
                {
                    m_selection_pos = -1;
                }

                m_cursor_pos = static_cast<i32>(m_value_temp.size());
            }
            else if (kb.is_button_pressed(Keyboard::Button::Backspace))
            {
                if (!this->delete_selection())
                {
                    if (m_cursor_pos > 0)
                    {
                        m_value_temp.erase(m_value_temp.begin() + m_cursor_pos - 1);
                        m_cursor_pos--;
                    }
                }
            }
            else if (kb.is_button_pressed(Keyboard::Button::Delete))
            {
                if (!this->delete_selection())
                {
                    if (m_cursor_pos < static_cast<i32>(m_value_temp.length()))
                        m_value_temp.erase(m_value_temp.begin() + m_cursor_pos);
                }
            }
            else if (kb.is_button_pressed(Keyboard::Button::Return))
            {
                if (!m_committed)
                    this->on_focus_lost();
            }
            else if (kb.is_button_pressed(Keyboard::Button::A) &&
                     kb.is_button_down(Keyboard::Button::LCtrl))
            {
                m_cursor_pos = static_cast<i32>(m_value_temp.length());
                m_selection_pos = 0;
            }
            else if (kb.is_button_pressed(Keyboard::Button::X) &&
                     kb.is_button_down(Keyboard::Button::LCtrl))
            {
                copy_selection();
                delete_selection();
            }
            else if (kb.is_button_pressed(Keyboard::Button::C) &&
                     kb.is_button_down(Keyboard::Button::LCtrl))
            {
                copy_selection();
            }
            else if (kb.is_button_pressed(Keyboard::Button::V) &&
                     kb.is_button_down(Keyboard::Button::LCtrl))
            {
                this->delete_selection();
                this->paste_from_clipboard();
            }

            m_valid_format = (m_value_temp == "") || this->check_format(m_value_temp, m_format);

            return true;
        }

        return false;
    }

    bool TextBox::on_key_released(const Keyboard& kb)
    {
        if (m_editable && this->focused())
            return true;

        return false;
    }

    bool TextBox::on_character_input(const Keyboard& kb)
    {
        if (m_editable && this->focused())
        {
            std::ostringstream convert;
            convert << kb.get_inputted_text();

            this->delete_selection();
            m_value_temp.insert(m_cursor_pos, convert.str());
            m_cursor_pos++;

            m_valid_format = m_value_temp.empty() || this->check_format(m_value_temp, m_format);

            return true;
        }

        return false;
    }

    bool TextBox::check_format(const std::string& input, const std::string& format)
    {
        if (format.empty())
            return true;
        try
        {
            std::regex regex(format);
            return regex_match(input, regex);
        }
        catch (const std::regex_error&)
        {
            throw;
        }
    }

    bool TextBox::copy_selection()
    {
        if (m_selection_pos > -1)
        {
            Window* window = this->window();
            if (window == nullptr)
                return false;

            i32 begin = m_cursor_pos;
            i32 end = m_selection_pos;

            if (begin > end)
                std::swap(begin, end);

            SDL3::SDL_SetClipboardText(m_value_temp.substr(begin, end).c_str());
            return true;
        }

        return false;
    }

    void TextBox::paste_from_clipboard()
    {
        Window* window = this->window();
        if (window == nullptr)
            return;

        if (SDL3::SDL_HasClipboardText())
        {
            const char* cbstr{ SDL3::SDL_GetClipboardText() };
            if (cbstr != nullptr)
                m_value_temp.insert(m_cursor_pos, std::string(cbstr));
        }
    }

    bool TextBox::delete_selection()
    {
        if (m_selection_pos > -1)
        {
            int begin = m_cursor_pos;
            int end = m_selection_pos;

            if (begin > end)
                std::swap(begin, end);

            if (begin == end - 1)
                m_value_temp.erase(m_value_temp.begin() + begin);
            else
                m_value_temp.erase(m_value_temp.begin() + begin, m_value_temp.begin() + end);

            m_cursor_pos = begin;
            m_selection_pos = -1;
            return true;
        }

        return false;
    }

    void TextBox::update_cursor(NVGcontext*, f32 lastx, const NVGglyphPosition* glyphs, int size)
    {
        // handle mouse cursor events
        if (m_mouse_down_pos.x != -1)
        {
            if ((m_mouse_down_modifier & Keyboard::Button::LShift) == 0)
                m_selection_pos = -1;
            else if (m_selection_pos == -1)
                m_selection_pos = m_cursor_pos;

            m_cursor_pos = this->position_to_cursor_index(m_mouse_down_pos.x, lastx, glyphs, size);
            m_mouse_down_pos = { -1, -1 };
        }
        else if (m_mouse_drag_pos.x != -1)
        {
            if (m_selection_pos == -1)
                m_selection_pos = m_cursor_pos;

            m_cursor_pos = this->position_to_cursor_index(m_mouse_drag_pos.x, lastx, glyphs, size);
        }
        else
        {
            // set cursor to last character
            if (m_cursor_pos == -2)
                m_cursor_pos = size;
        }

        if (m_cursor_pos == m_selection_pos)
            m_selection_pos = -1;
    }

    f32 TextBox::cursor_index_to_position(i32 index, f32 lastx, const NVGglyphPosition* glyphs,
                                          i32 size)
    {
        f32 pos{ 0.0f };

        if (index != size)
            pos = glyphs[index].x;
        else
        {
            // last character
            pos = lastx;
        }

        return pos;
    }

    int TextBox::position_to_cursor_index(f32 posx, f32 lastx, const NVGglyphPosition* glyphs,
                                          i32 size)
    {
        i32 m_cursor_id = 0;
        f32 caretx = glyphs[m_cursor_id].x;

        for (i32 j = 1; j < size; j++)
        {
            if (std::abs(caretx - posx) > std::abs(glyphs[j].x - posx))
            {
                m_cursor_id = j;
                caretx = glyphs[m_cursor_id].x;
            }
        }

        if (std::abs(caretx - posx) > std::abs(lastx - posx))
            m_cursor_id = size;

        return m_cursor_id;
    }

    TextBox::SpinArea TextBox::spin_area(ds::point<i32> pos) const
    {
        if ((0 <= pos.x - m_pos.x) && (pos.x - m_pos.x < 14.f))
        {
            // on scrolling arrows
            if (m_size.height >= pos.y - m_pos.y && pos.y - m_pos.y <= m_size.height / 2.0f)
            {
                // top part
                return SpinArea::Top;
            }
            else if (0.0f <= pos.y - m_pos.y && pos.y - m_pos.y > m_size.height / 2.0f)
            {
                // bottom part
                return SpinArea::Bottom;
            }
        }
        return SpinArea::None;
    }
}
