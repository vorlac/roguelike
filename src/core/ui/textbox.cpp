#include <iostream>
#include <regex>
#include <string>

#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "core/ui/textbox.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"
#include "utils/unicode.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_Clipboard.h>
SDL_C_LIB_END

namespace rl::ui {

    TextBox::TextBox(Widget* parent, const std::string& value)
        : Widget{ parent }
        , m_value{ value }
        , m_value_temp{ value }
    {
        if (m_theme != nullptr)
            m_font_size = m_theme->text_box_font_size;

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

    void TextBox::set_spinnable(const bool spinnable)
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

    void TextBox::set_alignment(const TextBox::Alignment align)
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

    void TextBox::set_units_image(const int image)
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

    void TextBox::set_editable(const bool editable)
    {
        m_editable = editable;
        this->set_cursor(editable ? Mouse::Cursor::IBeam : Mouse::Cursor::Arrow);
    }

    void TextBox::set_theme(Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme != nullptr)
            m_font_size = m_theme->text_box_font_size;
    }

    ds::dims<f32> TextBox::preferred_size() const
    {
        f32 uw{ 0.0f };
        ds::dims<f32> size{
            0.0f,
            this->font_size() * 1.4f,
        };

        auto&& context{ m_renderer->context() };
        if (m_units_image > 0)
        {
            ds::dims<f32> img_size{ 0.0f, 0.0f };
            nvg::image_size(context, m_units_image, &img_size.width, &img_size.height);
            f32 uh{ size.height * 0.4f };
            uw = img_size.width * uh / img_size.height;
        }
        else if (!m_units.empty())
            uw = nvg::text_bounds(context, 0, 0, m_units.c_str(), nullptr, nullptr);

        f32 sw{ 0.0f };
        if (m_spinnable)
            sw = 14.0f;

        f32 ts{ nvg::text_bounds(context, 0, 0, m_value.c_str(), nullptr, nullptr) };
        size.width = size.height + ts + uw + sw;

        return size;
    }

    void TextBox::draw()
    {
        Widget::draw();

        auto&& context{ m_renderer->context() };
        nvg::PaintStyle bg{ nvg::box_gradient(context, m_pos.x + 1.0f, m_pos.y + 1.0f + 1.0f,
                                              m_size.width - 2.0f, m_size.height - 2.0f, 3.0f, 4.0f,
                                              ds::color<f32>{ 255, 255, 255, 32 },
                                              ds::color<f32>{ 32, 32, 32, 32 }) };
        nvg::PaintStyle fg1{ nvg::box_gradient(context, m_pos.x + 1.0f, m_pos.y + 1.0f + 1.0f,
                                               m_size.width - 2.0f, m_size.height - 2.0f, 3.0f,
                                               4.0f, ds::color<f32>{ 150, 150, 150, 32 },
                                               ds::color<f32>{ 32, 32, 32, 32 }) };
        nvg::PaintStyle fg2{ nvg::box_gradient(context, m_pos.x + 1.0f, m_pos.y + 1.0f + 1.0f,
                                               m_size.width - 2.0f, m_size.height - 2.0f, 3.0f,
                                               4.0f, ds::color<f32>{ 255, 0, 0, 100 },
                                               ds::color<f32>{ 255, 0, 0, 50 }) };

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_pos.x + 1, m_pos.y + 1 + 1.0f, m_size.width - 2,
                          m_size.height - 2, 3);

        if (m_editable && this->focused())
            m_valid_format ? nvg::fill_paint(context, fg1) : nvg::fill_paint(context, fg2);
        else if (m_spinnable && m_mouse_down_pos.x != -1)
            nvg::fill_paint(context, fg1);
        else
            nvg::fill_paint(context, bg);

        nvg::fill(context);

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_pos.x + 0.5f, m_pos.y + 0.5f, m_size.width - 1,
                          m_size.height - 1, 2.5f);
        nvg::stroke_color(context, ds::color<f32>{ 0, 0, 0, 48 });
        nvg::stroke(context);

        nvg::font_size(context, this->font_size());
        nvg::font_face(context, Font::Name::Sans);

        ds::point<f32> draw_pos{
            m_pos.x,
            m_pos.y + m_size.height * 0.5f + 1,
        };

        f32 x_spacing{ m_size.height * 0.3f };
        f32 unit_width{ 0 };

        if (m_units_image > 0)
        {
            float w{ 0 };
            float h{ 0 };

            nvg::image_size(context, m_units_image, &w, &h);

            f32 unit_height{ m_size.height * 0.4f };
            unit_width = w * unit_height / h;

            nvg::PaintStyle img_paint{ nvg::image_pattern(
                context, m_pos.x + m_size.width - x_spacing - unit_width,
                draw_pos.y - unit_height * 0.5f, unit_width, unit_height, 0.0f, m_units_image,
                m_enabled ? 0.7f : 0.35f) };

            nvg::begin_path(context);
            nvg::rect(context, m_pos.x + m_size.width - x_spacing - unit_width,
                      draw_pos.y - unit_height * 0.5f, unit_width, unit_height);
            nvg::fill_paint(context, img_paint);
            nvg::fill(context);

            unit_width += 2;
        }
        else if (!m_units.empty())
        {
            unit_width = nvg::text_bounds(context, 0.0f, 0.0f, m_units.c_str(), nullptr, nullptr);

            ds::color<f32> color{ 255, 255, 255 };
            color.a = m_enabled ? 0.25f : 0.125f;

            nvg::fill_color(context, color);
            nvg::text_align(context, nvg::Align::NVGAlignRight | nvg::Align::NVGAlignMiddle);
            nvg::text(context, m_pos.x + m_size.width - x_spacing, draw_pos.y, m_units.c_str(),
                      nullptr);

            unit_width += 2;
        }

        f32 spin_arrows_width{ 0.0f };
        if (m_spinnable && !focused())
        {
            spin_arrows_width = 14.0f;

            nvg::font_face(context, "icons");
            nvg::font_size(context,
                           ((m_font_size < 0.0f) ? m_theme->button_font_size : m_font_size) *
                               this->icon_scale());

            bool spinning = m_mouse_down_pos.x != -1;

            {
                // up button
                bool hover{ m_mouse_focus && spin_area(m_mouse_pos) == SpinArea::Top };
                nvg::fill_color(context, (m_enabled && (hover || spinning))
                                             ? m_theme->text_color
                                             : m_theme->disabled_text_color);

                auto icon{ utf8(std::to_underlying(m_theme->text_box_up_icon)) };
                nvg::text_align(context, nvg::Align::NVGAlignLeft | nvg::Align::NVGAlignMiddle);

                ds::point<f32> icon_pos{
                    m_pos.x + 4.0f,
                    m_pos.y + m_size.height / 2.0f - x_spacing / 2.0f,
                };

                nvg::text(context, icon_pos.x, icon_pos.y, icon.data(), nullptr);
            }

            {
                // down button
                bool hover{ m_mouse_focus && this->spin_area(m_mouse_pos) == SpinArea::Bottom };
                nvg::fill_color(context, (m_enabled && (hover || spinning))
                                             ? m_theme->text_color
                                             : m_theme->disabled_text_color);

                auto&& icon{ utf8(m_theme->text_box_down_icon) };
                nvg::text_align(context, nvg::Align::NVGAlignLeft | nvg::Align::NVGAlignMiddle);

                ds::point<f32> icon_pos{
                    m_pos.x + 4.0f,
                    m_pos.y + (m_size.height / 2.0f) + (x_spacing / 2.0f + 1.5f),
                };

                nvg::text(context, icon_pos.x, icon_pos.y, icon.data());
            }

            nvg::font_size(context, this->font_size());
            nvg::font_face(context, Font::Name::Sans);
        }

        switch (m_alignment)
        {
            case Alignment::Left:
                nvg::text_align(context, nvg::Align::NVGAlignLeft | nvg::Align::NVGAlignMiddle);
                draw_pos.x += x_spacing + spin_arrows_width;
                break;
            case Alignment::Right:
                nvg::text_align(context, nvg::Align::NVGAlignRight | nvg::Align::NVGAlignMiddle);
                draw_pos.x += m_size.width - unit_width - x_spacing;
                break;
            case Alignment::Center:
                nvg::text_align(context, nvg::Align::NVGAlignCenter | nvg::Align::NVGAlignMiddle);
                draw_pos.x += m_size.width * 0.5f;
                break;
        }

        nvg::font_size(context, this->font_size());
        nvg::fill_color(context, m_enabled && (!m_committed || !m_value.empty())
                                     ? m_theme->text_color
                                     : m_theme->disabled_text_color);

        // clip visible text area
        f32 clip_x{ m_pos.x + x_spacing + spin_arrows_width - 1.0f };
        f32 clip_y{ m_pos.y + 1.0f };
        f32 clip_width{ m_size.width - unit_width - spin_arrows_width - 2.0f * x_spacing + 2.0f };
        f32 clip_height{ m_size.height - 3.0f };

        nvg::save(context);
        nvg::intersect_scissor(context, clip_x, clip_y, clip_width, clip_height);

        ds::point<f32> old_draw_pos{ draw_pos };
        draw_pos.x += static_cast<i32>(m_text_offset);

        if (m_committed)
        {
            nvg::text(context, draw_pos.x, draw_pos.y,
                      m_value.empty() ? m_placeholder.c_str() : m_value.c_str(), nullptr);
        }
        else
        {
            constexpr static i32 max_glyphs{ 1024 };
            nvg::GlyphPosition glyphs[max_glyphs];

            std::array<f32, 4> text_bound = { 0.0f };
            nvg::text_bounds(context, draw_pos.x, draw_pos.y, m_value_temp.c_str(), nullptr,
                             text_bound.data());

            f32 lineh{ text_bound[3] - text_bound[1] };

            // find cursor positions
            i32 nglyphs{ nvg::text_glyph_positions(context, draw_pos.x, draw_pos.y,
                                                   m_value_temp.c_str(), nullptr, glyphs,
                                                   max_glyphs) };
            this->update_cursor(text_bound[2], glyphs, nglyphs);

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
            nvg::text(context, draw_pos.x, draw_pos.y, m_value_temp.c_str(), nullptr);
            nvg::text_bounds(context, draw_pos.x, draw_pos.y, m_value_temp.c_str(), nullptr,
                             text_bound.data());

            // recompute cursor positions
            nglyphs = nvg::text_glyph_positions(context, draw_pos.x, draw_pos.y,
                                                m_value_temp.c_str(), nullptr, glyphs, max_glyphs);

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
                    nvg::begin_path(context);
                    nvg::fill_color(context, ds::color<f32>{ 255, 255, 255, 80 });
                    nvg::rect(context, caretx, draw_pos.y - lineh * 0.5f, selx - caretx, lineh);
                    nvg::fill(context);
                }

                f32 caretx{ cursor_index_to_position(m_cursor_pos, text_bound[2], glyphs, nglyphs) };

                // draw cursor
                nvg::begin_path(context);
                nvg::move_to(context, caretx, draw_pos.y - lineh * 0.5f);
                nvg::line_to(context, caretx, draw_pos.y + lineh * 0.5f);
                nvg::stroke_color(context, ds::color<f32>{ 255, 192, 0, 255 });
                nvg::stroke_width(context, 1.0f);
                nvg::stroke(context);
            }
        }

        nvg::restore(context);
    }

    bool TextBox::on_mouse_entered(const Mouse& mouse)
    {
        Widget::on_mouse_entered(mouse);
        return true;
    }

    bool TextBox::on_mouse_exited(const Mouse& mouse)
    {
        Widget::on_mouse_exited(mouse);
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
            if (kb.is_button_down(Keyboard::Scancode::LCtrl))
                m_mouse_down_modifier |= Keyboard::Scancode::LCtrl;
            if (kb.is_button_down(Keyboard::Scancode::LShift))
                m_mouse_down_modifier |= Keyboard::Scancode::LShift;

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
                if (kb.is_button_down(Keyboard::Scancode::LCtrl))
                    m_mouse_down_modifier |= Keyboard::Scancode::LCtrl;
                if (kb.is_button_down(Keyboard::Scancode::LShift))
                    m_mouse_down_modifier |= Keyboard::Scancode::LShift;

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

    bool TextBox::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        m_mouse_pos = mouse.pos();
        m_mouse_drag_pos = mouse.pos();
        return m_editable && this->focused();
    }

    bool TextBox::on_focus_gained()
    {
        Widget::on_focus_gained();
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
        Widget::on_focus_lost();
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
            if (kb.is_button_pressed(Keyboard::Scancode::Left))
            {
                if (kb.is_button_down(Keyboard::Scancode::LShift))
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
            else if (kb.is_button_pressed(Keyboard::Scancode::Right))
            {
                if (kb.is_button_down(Keyboard::Scancode::Shift))
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
            else if (kb.is_button_pressed(Keyboard::Scancode::Home))
            {
                if (kb.is_button_down(Keyboard::Scancode::Shift))
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
            else if (kb.is_button_pressed(Keyboard::Scancode::End))
            {
                if (kb.is_button_down(Keyboard::Scancode::Shift))
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
            else if (kb.is_button_pressed(Keyboard::Scancode::Backspace))
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
            else if (kb.is_button_pressed(Keyboard::Scancode::Delete))
            {
                if (!this->delete_selection())
                {
                    if (m_cursor_pos < static_cast<i32>(m_value_temp.length()))
                        m_value_temp.erase(m_value_temp.begin() + m_cursor_pos);
                }
            }
            else if (kb.is_button_pressed(Keyboard::Scancode::Return))
            {
                if (!m_committed)
                    this->on_focus_lost();
            }
            else if (kb.is_button_pressed(Keyboard::Scancode::A) &&
                     kb.is_button_down(Keyboard::Scancode::LCtrl))
            {
                m_cursor_pos = static_cast<i32>(m_value_temp.length());
                m_selection_pos = 0;
            }
            else if (kb.is_button_pressed(Keyboard::Scancode::X) &&
                     kb.is_button_down(Keyboard::Scancode::LCtrl))
            {
                copy_selection();
                delete_selection();
            }
            else if (kb.is_button_pressed(Keyboard::Scancode::C) &&
                     kb.is_button_down(Keyboard::Scancode::LCtrl))
            {
                copy_selection();
            }
            else if (kb.is_button_pressed(Keyboard::Scancode::V) &&
                     kb.is_button_down(Keyboard::Scancode::LCtrl))
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
            Dialog* dialog{ this->dialog() };
            if (dialog == nullptr)
                return false;

            i32 begin{ m_cursor_pos };
            i32 end{ m_selection_pos };

            if (begin > end)
                std::swap(begin, end);

            SDL3::SDL_SetClipboardText(m_value_temp.substr(begin, end).c_str());
            return true;
        }

        return false;
    }

    void TextBox::paste_from_clipboard()
    {
        Dialog* dialog{ this->dialog() };
        if (dialog == nullptr)
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

    void TextBox::update_cursor(const f32 last_x, const nvg::GlyphPosition* glyphs, const i32 size)
    {
        // handle mouse cursor events
        if (m_mouse_down_pos.x != -1)
        {
            if ((m_mouse_down_modifier & Keyboard::Scancode::LShift) == 0)
                m_selection_pos = -1;
            else if (m_selection_pos == -1)
                m_selection_pos = m_cursor_pos;

            m_cursor_pos = this->position_to_cursor_index(m_mouse_down_pos.x, last_x, glyphs, size);
            m_mouse_down_pos = { -1, -1 };
        }
        else if (m_mouse_drag_pos.x != -1)
        {
            if (m_selection_pos == -1)
                m_selection_pos = m_cursor_pos;

            m_cursor_pos = this->position_to_cursor_index(m_mouse_drag_pos.x, last_x, glyphs, size);
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

    f32 TextBox::cursor_index_to_position(const i32 index, const f32 last_x,
                                          const nvg::GlyphPosition* glyphs, const i32 size)
    {
        f32 pos{ 0.0f };

        if (index != size)
            pos = glyphs[index].x;
        else
        {
            // last character
            pos = last_x;
        }

        return pos;
    }

    i32 TextBox::position_to_cursor_index(const f32 pos_x, const f32 last_x,
                                          const nvg::GlyphPosition* glyphs, const i32 size)
    {
        i32 m_cursor_id{ 0 };
        f32 caretx{ glyphs[m_cursor_id].x };

        for (i32 j = 1; j < size; j++)
        {
            if (std::abs(caretx - pos_x) > std::abs(glyphs[j].x - pos_x))
            {
                m_cursor_id = j;
                caretx = glyphs[m_cursor_id].x;
            }
        }

        if (std::abs(caretx - pos_x) > std::abs(last_x - pos_x))
            m_cursor_id = size;

        return m_cursor_id;
    }

    TextBox::SpinArea TextBox::spin_area(const ds::point<f32>& pos) const
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
