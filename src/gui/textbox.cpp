#include <cmath>
#include <iostream>
#include <mutex>
#include <regex>
#include <thread>

#include "gui/entypo.hpp"
#include "gui/nanovg.h"
#include "gui/screen.hpp"
#include "gui/textbox.hpp"
#include "gui/theme.hpp"
#include "gui/window.hpp"
#include "sdl/defs.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

namespace rl::gui {
    struct TextBox::AsyncTexture
    {
        int id;
        Texture tex;
        NVGcontext* ctx = nullptr;
        float value = -1;

        AsyncTexture(int tex_id)
            : id(tex_id)
        {
        }

        void load(TextBox* ptr, bool editable, bool focused, bool validFormat, bool outside)
        {
            TextBox* tbox = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                Theme* m_theme = tbox->theme();
                std::lock_guard<std::mutex> guard(m_theme->m_load_mutex);

                int ww = tbox->width();
                int hh = tbox->height();
                int realw = ww + 2;
                int realh = hh + 2;
                int dx = 1, dy = 1;
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, realw, realh + 2, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, realw, realh, pxRatio);

                NVGpaint bg = nvgBoxGradient(ctx, dx + 1, dy + 1 + 1.0f, ww - 2, hh - 2, 3, 4,
                                             Color(255, 128).to_nvg_color(),
                                             Color(32, 32).to_nvg_color());
                NVGpaint fg1 = nvgBoxGradient(ctx, dx + 1, dy + 1 + 1.0f, ww - 2, hh - 2, 3, 4,
                                              Color(150, 32).to_nvg_color(),
                                              Color(32, 32).to_nvg_color());
                NVGpaint fg2 = nvgBoxGradient(ctx, dx + 1, dy + 1 + 1.0f, ww - 2, hh - 2, 3, 4,
                                              nvgRGBA(255, 0, 0, 100), nvgRGBA(255, 0, 0, 50));

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, dx + 1, dy + 1 + 1.0f, ww - 2, hh - 2, 3);

                if (editable && focused)
                    validFormat ? nvgFillPaint(ctx, fg1) : nvgFillPaint(ctx, fg2);
                else if (outside)
                    nvgFillPaint(ctx, fg1);
                else
                    nvgFillPaint(ctx, bg);

                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, dx + 0.5f, dy + 0.5f, ww - 1, hh - 1, 2.5f);
                nvgStrokeColor(ctx, Color(0, 48).to_nvg_color());
                nvgStroke(ctx);

                nvgEndFrame(ctx);
                self->tex.rrect = { 0, 0, realw, realh };
                self->ctx = ctx;
            });

            tgr.detach();
        }

        void perform(SDL3::SDL_Renderer* renderer)
        {
            if (!ctx)
                return;

            unsigned char* rgba = nvgReadPixelsRT(ctx);

            if (tex.tex)
            {
                int w, h;
                SDL_QueryTexture(tex.tex, nullptr, nullptr, &w, &h);
                if (w != tex.w() || h != tex.h())
                    SDL3::SDL_DestroyTexture(tex.tex);
            }

            if (!tex.tex)
                tex.tex = SDL3::SDL_CreateTexture(renderer, SDL3::SDL_PIXELFORMAT_ABGR8888,
                                                  SDL3::SDL_TEXTUREACCESS_STREAMING, tex.w(),
                                                  tex.h());

            int pitch;
            uint8_t* pixels;
            int ok = SDL3::SDL_LockTexture(tex.tex, nullptr, (void**)&pixels, &pitch);
            memcpy(pixels, rgba, sizeof(uint32_t) * tex.w() * tex.h());
            SDL3::SDL_SetTextureBlendMode(tex.tex, SDL3::SDL_BLENDMODE_BLEND);
            SDL3::SDL_UnlockTexture(tex.tex);

            nvgDeleteRT(ctx);
            ctx = nullptr;
        }
    };

    TextBox::TextBox(Widget* parent, const std::string& value, const std::string& units)
        : Widget(parent)
        , m_editable(false)
        , m_spinnable(false)
        , m_committed(true)
        , m_value(value)
        , m_default_value("")
        , m_alignment(Alignment::Center)
        , m_units(units)
        , m_format("")
        , m_units_image(-1)
        , m_valid_format(true)
        , m_value_temp(value)
        , m_cursor_pos(-1)
        , m_selection_pos(-1)
        , m_mouse_pos(Vector2i(-1, -1))
        , m_mouse_down_pos(Vector2i(-1, -1))
        , m_mouse_drag_pos(Vector2i(-1, -1))
        , m_mouse_down_modifier(0)
        , m_text_offset(0)
        , m_last_click(0)
    {
        if (m_theme)
            m_font_size = m_theme->m_text_box_font_size;
        m_caption_texture.dirty = true;
        m_units_texture.dirty = true;
    }

    void TextBox::set_editable(bool editable)
    {
        m_editable = editable;
        m_caption_texture.dirty = true;
        this->set_cursor(editable ? Cursor::IBeam : Cursor::Arrow);
    }

    void TextBox::set_theme(Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme)
            m_font_size = m_theme->m_text_box_font_size;
    }

    Vector2i TextBox::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        Vector2i size(0, font_size() * 1.4f);

        float uw = 0;
        if (m_units_image > 0)
        {
            /*  int w, h;
              nvgImageSize(ctx, m_units_image, &w, &h);
              float uh = size(1) * 0.4f;
              uw = w * uh / h;
              */
        }
        else if (!m_units.empty())
        {
            uw = const_cast<TextBox*>(this)->m_theme->get_utf8_width("sans", font_size(),
                                                                     m_units.c_str());
        }
        float sw = 0;
        if (m_spinnable)
            sw = 14.f;

        float ts = const_cast<TextBox*>(this)->m_theme->get_utf8_width("sans", font_size(),
                                                                       m_value.c_str());
        size.x = size.y + ts + uw + sw;
        return size;
    }

    void TextBox::draw_body(SDL3::SDL_Renderer* renderer)
    {
        bool outside = m_spinnable && m_mouse_down_pos.x != -1;
        int id = (m_editable ? 0x1 : 0) + (focused() ? 0x2 : 0) + (m_valid_format ? 0x4 : 0) +
                 (outside ? 0x8 : 0);

        auto atx = std::find_if(m_textures.begin(), m_textures.end(),
                                [id](const TextBox::AsyncTexturePtr& p) {
                                    return p->id == id;
                                });

        if (atx != m_textures.end())
            draw_texture(*atx, renderer);
        else
        {
            TextBox::AsyncTexturePtr newtx = std::make_shared<TextBox::AsyncTexture>(id);
            newtx->load(this, m_editable, focused(), m_valid_format, outside);
            m_textures.push_back(newtx);

            draw_texture(m_curr_texture, renderer);
        }
    }

    void TextBox::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        SDL3::SDL_Point ap = get_absolute_pos();

        draw_body(renderer);

        Vector2i drawPos = absolute_position();
        float unitWidth = 0;

        if (m_units_image > 0)
        {
            // int w, h;
            // nvgImageSize(ctx, m_units_image, &w, &h);
            // float unitHeight = m_size.y() * 0.4f;
            // unitWidth = w * unitHeight / h;
            // NVGpaint imgPaint = nvgImagePattern(
            //     ctx, mPos.x() + m_size.x() - xSpacing - unitWidth, drawPos.y() - unitHeight *
            //     0.5f, unitWidth, unitHeight, 0, m_units_image, mEnabled ? 0.7f : 0.35f);
            // nvgBeginPath(ctx);
            // nvgRect(ctx, mPos.x() + m_size.x() - xSpacing - unitWidth,
            //         drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight);
            // nvgFillPaint(ctx, imgPaint);
            // nvgFill(ctx);
            // unitWidth += 2;
        }
        else if (!m_units.empty())
        {
            if (m_units_texture.dirty)
                m_theme->get_texture_and_rect_utf8(renderer, m_units_texture, 0, 0, m_units.c_str(),
                                                   "sans", font_size(),
                                                   Color(255, m_enabled ? 64 : 32));

            unitWidth = m_units_texture.w() + 2;
            SDL_RenderCopy(renderer, m_units_texture,
                           absolute_position() + Vector2i(m_size.x - unitWidth,
                                                          (m_size.y - m_units_texture.h()) * 0.5f));
            unitWidth += (2 + 2);
        }

        float spinArrowsWidth = 0.f;

        // if (mSpinnable && !focused())
        // {
        //     spinArrowsWidth = 14.f;
        //
        //     nvgFontFace(ctx, "icons");
        //     nvgFontSize(ctx, ((mFontSize < 0) ? m_theme->m_button_font_size : mFontSize) * 1.2f);
        //
        //     bool spinning = m_mouse_down_pos.x() != -1;
        //     {
        //         bool hover = mMouseFocus && spin_area(m_mouse_pos) == SpinArea::Top;
        //         nvgFillColor(ctx, (mEnabled && (hover || spinning)) ? m_theme->m_text_color
        //                                                             :
        //                                                             m_theme->m_disabled_text_color);
        //         auto icon = utf8(ENTYPO_ICON_CHEVRON_UP);
        //         nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        //         Vector2f iconPos(mPos.x() + 4.f, mPos.y() + m_size.y() / 2.f - xSpacing / 2.f);
        //         nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
        //     }
        //
        //     {
        //         bool hover = mMouseFocus && spin_area(m_mouse_pos) == SpinArea::Bottom;
        //         nvgFillColor(ctx, (mEnabled && (hover || spinning)) ? m_theme->m_text_color
        //                                                             :
        //                                                             m_theme->m_disabled_text_color);
        //         auto icon = utf8(ENTYPO_ICON_CHEVRON_DOWN);
        //         nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        //         Vector2f iconPos(mPos.x() + 4.f, mPos.y() + m_size.y() / 2.f + xSpacing / 2.f
        //         + 1.5f); nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
        //     }
        //
        //     nvgFontSize(ctx, font_size());
        //     nvgFontFace(ctx, "sans");
        // }

        float xSpacing = 3.f;
        switch (m_alignment)
        {
            case Alignment::Left:
                drawPos.x = get_absolute_left() + xSpacing + spinArrowsWidth;
                break;
            case Alignment::Right:
                drawPos.x = get_absolute_left() + m_size.x - m_caption_texture.w() - unitWidth -
                            xSpacing;
                break;
            case Alignment::Center:
                if (m_units.empty())
                    drawPos.x = get_absolute_left() + m_size.x * 0.5f;
                else
                    drawPos.x = get_absolute_left() + m_size.x * 0.3f;
                break;
        }

        // clip visible text area
        float clipX = m_pos.x + spinArrowsWidth - 1.0f;
        float clipY = m_pos.y + 1.0f;
        float clipWidth = m_size.x - unitWidth - spinArrowsWidth + 2.0f;
        float clipHeight = m_size.y - 3.0f;

        Vector2f oldDrawPos(drawPos.x, drawPos.y);
        drawPos.x += m_text_offset;
        drawPos.y += (m_size.y - m_caption_texture.h()) / 2;

        if (m_caption_texture.dirty)
            m_theme->get_texture_and_rect_utf8(
                renderer, m_caption_texture, 0, 0, m_value.c_str(), "sans", font_size(),
                m_enabled ? m_theme->m_text_color : m_theme->m_disabled_text_color);

        if (m_committed)
        {
            SDL3::SDL_FRect drawRect{ static_cast<float>(drawPos.x), static_cast<float>(drawPos.y) };
            SDL3::SDL_RenderTexture(renderer, m_caption_texture.tex, &drawRect, nullptr);
        }
        else
        {
            int w, h;
            m_theme->get_utf8_bounds("sans", font_size(), m_value_temp.c_str(), &w, &h);
            float textBound[4] = { (float)drawPos.x, (float)drawPos.y, (float)(drawPos.x + w),
                                   (float)(drawPos.y + h) };
            float lineh = textBound[3] - textBound[1];

            // find cursor positions
            update_cursor(textBound[2], m_value_temp);

            // compute text offset
            int prevCPos = m_cursor_pos > 0 ? m_cursor_pos - 1 : 0;
            int nextCPos = m_cursor_pos < (int)m_value_temp.size() ? m_cursor_pos + 1
                                                                   : (int)m_value_temp.size();
            float prevCX = cursor_idx_to_position(prevCPos, textBound[2], m_value_temp);
            float nextCX = cursor_idx_to_position(nextCPos, textBound[2], m_value_temp);

            if (nextCX > clipX + clipWidth)
                m_text_offset -= nextCX - (clipX + clipWidth) + 1;
            if (prevCX < clipX)
                m_text_offset += clipX - prevCX + 1;

            // drawPos.x() = oldDrawPos.x() + m_text_offset;

            if (m_temp_texture.dirty)
                m_theme->get_texture_and_rect_utf8(renderer, m_temp_texture, 0, 0,
                                                   m_value_temp.c_str(), "sans", font_size(),
                                                   m_theme->m_text_color);

            // draw text with offset
            SDL3::SDL_FRect oldDrawRect{ static_cast<float>(drawPos.x),
                                         static_cast<float>(drawPos.y) };
            SDL3::SDL_RenderTexture(renderer, m_temp_texture.tex, &oldDrawRect, nullptr);

            if (m_cursor_pos > -1)
            {
                if (m_selection_pos > -1)
                {
                    float caretx = cursor_idx_to_position(m_cursor_pos, textBound[2], m_value_temp);
                    float selx = cursor_idx_to_position(m_selection_pos, textBound[2], m_value_temp);

                    if (caretx > selx)
                        std::swap(caretx, selx);

                    // draw selection
                    SDL3::SDL_Color c = Color(255, 255, 255, 80).sdl_color();
                    SDL3::SDL_FRect sr{
                        std::round(oldDrawPos.x + caretx),
                        oldDrawPos.y + 4.0f,
                        std::round(selx - caretx),
                        height() - 4.0f,
                    };
                    SDL3::SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
                    SDL3::SDL_RenderFillRect(renderer, &sr);
                }

                caret_last_tick_count = SDL3::SDL_GetTicks();
                // draw cursor
                if (caret_last_tick_count % 1000 < 500)
                {
                    float caretx = cursor_idx_to_position(m_cursor_pos, textBound[2], m_value_temp);

                    SDL3::SDL_Color c = Color(255, 192, 0, 255).sdl_color();
                    SDL3::SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
                    SDL3::SDL_RenderLine(renderer, oldDrawPos.x + caretx, oldDrawPos.y + 4,
                                         oldDrawPos.x + caretx, oldDrawPos.y + lineh - 3);
                }
            }
        }
    }

    bool TextBox::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (button == SDL_BUTTON_LEFT && down && !m_focused)
        {
            if (!m_spinnable || spin_area(p) == SpinArea::None) /* not on scrolling arrows */
                request_focus();
        }

        if (m_editable && focused())
        {
            if (down)
            {
                m_mouse_down_pos = p;
                m_mouse_down_modifier = modifiers;

                double time = SDL3::SDL_GetTicks() / double(SDL_MS_PER_SECOND);
                if (time - m_last_click < 0.25)
                {
                    /* Double-click: select all text */
                    m_selection_pos = 0;
                    m_cursor_pos = (int)m_value_temp.size();
                    m_mouse_down_pos = Vector2i{ -1, -1 };
                }
                m_last_click = time;
            }
            else
            {
                m_mouse_down_pos = Vector2i{ -1, -1 };
                m_mouse_drag_pos = Vector2i{ -1, -1 };
            }
            return true;
        }
        else if (m_spinnable && !focused())
        {
            if (down)
            {
                if (spin_area(p) == SpinArea::None)
                {
                    m_mouse_down_pos = p;
                    m_mouse_down_modifier = modifiers;

                    double time = SDL3::SDL_GetTicks() / double(SDL_MS_PER_SECOND);
                    if (time - m_last_click < 0.25)
                    {
                        /* Double-click: reset to default value */
                        m_value = m_default_value;
                        if (m_callback)
                            m_callback(m_value);

                        m_mouse_down_pos = Vector2i{ -1, -1 };
                    }
                    m_last_click = time;
                }
                else
                {
                    m_mouse_down_pos = Vector2i{ -1, -1 };
                    m_mouse_drag_pos = Vector2i{ -1, -1 };
                }
            }
            else
            {
                m_mouse_down_pos = Vector2i{ -1, -1 };
                m_mouse_drag_pos = Vector2i{ -1, -1 };
            }
            return true;
        }

        return false;
    }

    bool TextBox::mouse_motion_event(const Vector2i& p, const Vector2i& /* rel */, int /* button */,
                                     int /* modifiers */)
    {
        m_mouse_pos = p;

        if (!m_editable)
            set_cursor(Cursor::Arrow);
        else if (m_spinnable && !focused() && spin_area(m_mouse_pos) != SpinArea::None) /* scrolling
                                                                                         * arrows
                                                                                         */
            set_cursor(Cursor::Hand);
        else
            set_cursor(Cursor::IBeam);

        if (m_editable && focused())
            return true;
        return false;
    }

    bool TextBox::mouse_drag_event(const Vector2i& p, const Vector2i& /* rel */, int /* button */,
                                   int /* modifiers */)
    {
        m_mouse_pos = p;
        m_mouse_drag_pos = p;

        if (m_editable && focused())
            return true;
        return false;
    }

    bool TextBox::focus_event(bool focused)
    {
        Widget::focus_event(focused);

        std::string backup = m_value;

        if (m_editable)
        {
            if (focused)
            {
                m_value_temp = m_value;
                m_temp_texture.dirty = true;
                m_committed = false;
                m_cursor_pos = 0;
            }
            else
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

                m_valid_format = true;
                m_caption_texture.dirty = true;
                m_committed = true;
                m_cursor_pos = -1;
                m_selection_pos = -1;
                m_text_offset = 0;
            }

            m_valid_format = (m_value_temp == "") || check_format(m_value_temp, m_format);
        }

        return true;
    }

    bool TextBox::kb_button_event(int key, int /* scancode */, int action, int modifiers)
    {
        if (m_editable && focused())
        {
            if (action == SDL_PRESSED)
            {
                if (key == SDL3::SDLK_LEFT)
                {
                    if (modifiers & SDL3::SDL_KMOD_SHIFT)
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
                else if (key == SDL3::SDLK_RIGHT)
                {
                    if (modifiers & SDL3::SDL_KMOD_SHIFT)
                    {
                        if (m_selection_pos == -1)
                            m_selection_pos = m_cursor_pos;
                    }
                    else
                    {
                        m_selection_pos = -1;
                    }

                    if (m_cursor_pos < (int)m_value_temp.length())
                        m_cursor_pos++;
                }
                else if (key == SDL3::SDLK_HOME)
                {
                    if (modifiers & SDL3::SDL_KMOD_SHIFT)
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
                else if (key == SDL3::SDLK_END)
                {
                    if (modifiers & SDL3::SDL_KMOD_SHIFT)
                    {
                        if (m_selection_pos == -1)
                            m_selection_pos = m_cursor_pos;
                    }
                    else
                    {
                        m_selection_pos = -1;
                    }

                    m_cursor_pos = (int)m_value_temp.size();
                }
                else if (key == SDL3::SDLK_BACKSPACE)
                {
                    if (!delete_selection())
                    {
                        if (m_cursor_pos > 0)
                        {
                            m_value_temp.erase(m_value_temp.begin() + m_cursor_pos - 1);
                            m_temp_texture.dirty = true;
                            m_cursor_pos--;
                        }
                    }
                }
                else if (key == SDL3::SDLK_DELETE)
                {
                    if (!delete_selection())
                    {
                        if (m_cursor_pos < (int)m_value_temp.length())
                            m_value_temp.erase(m_value_temp.begin() + m_cursor_pos);
                        m_temp_texture.dirty = true;
                    }
                }
                else if (key == SDL3::SDLK_RETURN)
                {
                    if (!m_committed)
                        focus_event(false);
                }
                else if (key == SDL3::SDLK_a && modifiers & SDL3::SDLK_LCTRL)
                {
                    m_cursor_pos = (int)m_value_temp.length();
                    m_selection_pos = 0;
                }
                else if (key == SDL3::SDLK_x && modifiers & SDL3::SDLK_LCTRL)
                {
                    copy_selection();
                    delete_selection();
                }
                else if (key == SDL3::SDLK_c && modifiers & SDL3::SDLK_LCTRL)
                {
                    copy_selection();
                }
                else if (key == SDL3::SDLK_v && modifiers & SDL3::SDLK_LCTRL)
                {
                    delete_selection();
                    paste_from_clipboard();
                }

                m_valid_format = (m_value_temp == "") || check_format(m_value_temp, m_format);
            }
            return true;
        }

        return false;
    }

    bool TextBox::kb_character_event(unsigned int codepoint)
    {
        if (m_editable && focused())
        {
            std::ostringstream convert;
            convert << (char)codepoint;

            delete_selection();
            m_value_temp.insert(m_cursor_pos, convert.str());
            m_cursor_pos++;

            m_valid_format = (m_value_temp == "") || check_format(m_value_temp, m_format);
            m_temp_texture.dirty = true;

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
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
            std::cerr
                << "Warning: cannot validate text field due to lacking regular expression support. please compile with GCC >= 4.9"
                << std::endl;
            return true;
#else
            throw;
#endif
        }
    }

    bool TextBox::copy_selection()
    {
        if (m_selection_pos > -1)
        {
            Screen* sc = dynamic_cast<Screen*>(this->window()->parent());

            int begin = m_cursor_pos;
            int end = m_selection_pos;

            if (begin > end)
                std::swap(begin, end);

            SDL3::SDL_SetClipboardText(m_value_temp.substr(begin, end).c_str());
            return true;
        }

        return false;
    }

    void TextBox::paste_from_clipboard()
    {
        Screen* sc = dynamic_cast<Screen*>(this->window()->parent());
        const char* cbstr = SDL3::SDL_GetClipboardText();
        if (cbstr)
        {
            m_value_temp.insert(m_cursor_pos, std::string(cbstr));
            m_caption_texture.dirty = true;
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
            m_temp_texture.dirty = true;

            return true;
        }

        return false;
    }

    void TextBox::update_cursor(float lastx, const std::string& str)
    {
        // handle mouse cursor events
        if (m_mouse_down_pos.x != -1)
        {
            if (m_mouse_down_modifier == SDL3::SDL_KMOD_SHIFT)
            {
                if (m_selection_pos == -1)
                    m_selection_pos = m_cursor_pos;
            }
            else
                m_selection_pos = -1;

            m_cursor_pos = position_to_cursor_idx(m_mouse_down_pos.x, lastx, str);

            m_mouse_down_pos = Vector2i{ -1, -1 };
        }
        else if (m_mouse_drag_pos.x != -1)
        {
            if (m_selection_pos == -1)
                m_selection_pos = m_cursor_pos;

            m_cursor_pos = position_to_cursor_idx(m_mouse_drag_pos.x, lastx, str);
        }
        else
        {
            // set cursor to last character
            if (m_cursor_pos == -2)
                m_cursor_pos = (int)str.size();
        }

        if (m_cursor_pos == m_selection_pos)
            m_selection_pos = -1;
    }

    float TextBox::cursor_idx_to_position(int index, float lastx, const std::string& str)
    {
        float pos = 0;
        if (index >= str.size())
            pos = m_temp_texture.w();  // last character
        else
            pos = m_theme->get_utf8_width("sans", font_size(), str.substr(0, index).c_str());
        ;

        return pos;
    }

    int TextBox::position_to_cursor_idx(float posx, float lastx, const std::string& str)
    {
        int mCursorId = 0;
        float caretx = m_theme->get_utf8_width("sans", font_size(),
                                               str.substr(0, mCursorId).c_str());
        for (int j = 1; j <= str.size(); j++)
        {
            int glposx = m_theme->get_utf8_width("sans", font_size(), str.substr(0, j).c_str());
            if (std::abs(caretx - posx) > std::abs(glposx - posx))
            {
                mCursorId = j;
                caretx = m_theme->get_utf8_width("sans", font_size(),
                                                 str.substr(0, mCursorId).c_str());
            }
        }
        if (std::abs(caretx - posx) > std::abs(lastx - posx))
            mCursorId = (int)str.size();

        return mCursorId;
    }

    TextBox::SpinArea TextBox::spin_area(const Vector2i& pos)
    {
        if (0 <= pos.x - m_pos.x && pos.x - m_pos.x < 14.f)
        { /* on scrolling arrows */
            if (m_size.y >= pos.y - m_pos.y && pos.y - m_pos.y <= m_size.y / 2.f)
            { /* top part */
                return SpinArea::Top;
            }
            else if (0.f <= pos.y - m_pos.y && pos.y - m_pos.y > m_size.y / 2.f)
            { /* bottom part */
                return SpinArea::Bottom;
            }
        }
        return SpinArea::None;
    }

    void TextBox::draw_texture(TextBox::AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer)
    {
        if (texture)
        {
            texture->perform(renderer);

            if (texture->tex.tex)
            {
                auto&& pos = absolute_position().tofloat();
                SDL3::SDL_FRect rect{ pos.x, pos.y };
                SDL3::SDL_RenderTexture(renderer, texture->tex.tex, &rect, nullptr);

                if (!m_curr_texture || texture->id != m_curr_texture->id)
                    m_curr_texture = texture;
            }
            else if (m_curr_texture)
            {
                auto&& pos = absolute_position().tofloat();
                SDL3::SDL_FRect rect{ pos.x, pos.y };
                SDL3::SDL_RenderTexture(renderer, m_curr_texture->tex.tex, &rect, nullptr);
            }
        }
    }

}
