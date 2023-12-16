#include <array>
#include <mutex>
#include <thread>

#include "gui/button.hpp"
#include "gui/nanovg.h"
#include "gui/theme.hpp"
#include "sdl/defs.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

namespace rl::gui {
    struct Button::AsyncTexture
    {
        int id{};
        Texture tex{};
        NVGcontext* ctx{ nullptr };

        AsyncTexture(int tex_id)
            : id(tex_id)
        {
        }

        ~AsyncTexture() = default;

        void load(Button* ptr)
        {
            Button* button{ ptr };
            AsyncTexture* self{ this };
            std::thread tgr([=]() {
                std::lock_guard<std::mutex> lock(button->theme()->m_load_mutex);

                NVGcontext* ctx = nullptr;
                int realw, realh;
                button->render_body_texture(ctx, realw, realh);
                self->tex.rrect = { 0, 0, realw, realh };
                self->ctx = ctx;
            });

            tgr.detach();
        }

        void perform(const std::unique_ptr<rl::Renderer>& renderer)
        {
            if (ctx == nullptr)
                return;

            u8* rgba = nvgReadPixelsRT(ctx);
            // TODO
        }

        void perform(SDL3::SDL_Renderer* renderer)
        {
            if (!ctx)
                return;

            unsigned char* rgba = nvgReadPixelsRT(ctx);

            tex.tex = SDL3::SDL_CreateTexture(renderer, SDL3::SDL_PIXELFORMAT_ABGR8888,
                                              SDL3::SDL_TEXTUREACCESS_STREAMING, tex.w(), tex.h());

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

    Button::Button(Widget* parent, const std::string& caption, int icon)
        : Widget(parent)
        , m_caption(caption)
        , m_icon(icon)
        , m_icon_position(IconPosition::LeftCentered)
        , m_pushed(false)
        , m_flags(NormalButton)
        , m_background_color(Color(0, 0))
        , m_text_color(Color(0, 0))
    {
        m_caption_texture.dirty = true;
        m_icon_texture.dirty = true;
    }

    Vector2i Button::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        int font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
        float tw = const_cast<Button*>(this)->m_theme->get_text_width("sans-bold", font_size,
                                                                      m_caption.c_str());
        float iw = 0.0f, ih = font_size;

        if (m_icon)
        {
            if (nvg_is_font_icon(m_icon))
            {
                ih *= 1.5f;
                iw = const_cast<Button*>(this)->m_theme->get_utf8_width("icons", ih,
                                                                        utf8(m_icon).data()) +
                     m_size.y * 0.15f;
            }
            else
            {
                int w{ 0 };
                int h{ 0 };
                ih *= 0.9f;

                SDL3::SDL_QueryTexture(reinterpret_cast<SDL3::SDL_Texture*>(m_icon), nullptr,
                                       nullptr, &w, &h);
                iw = w * ih / h;
            }
        }
        return Vector2i((int)(tw + iw) + 20, font_size + 10);
    }

    bool Button::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        Widget::mouse_button_event(p, button, down, modifiers);

        // Temporarily increase the reference count of the button in
        // case the button causes the parent window to be destructed
        refcounted<Button> self = this;

        if (button == SDL_BUTTON_LEFT && m_enabled)
        {
            bool pushedBackup = m_pushed;
            if (down)
            {
                if (m_flags & Button::Flags::RadioButton)
                {
                    if (m_button_group.empty())
                    {
                        for (auto widget : parent()->children())
                        {
                            Button* b = dynamic_cast<Button*>(widget);
                            if (b != this && b && (b->flags() & Button::Flags::RadioButton) &&
                                b->m_pushed)
                            {
                                b->m_pushed = false;
                                if (b->m_change_callback)
                                    b->m_change_callback(false);
                            }
                        }
                    }
                    else
                    {
                        for (auto b : m_button_group)
                        {
                            if (b != this && (b->flags() & Button::Flags::RadioButton) &&
                                b->m_pushed)
                            {
                                b->m_pushed = false;
                                if (b->m_change_callback)
                                    b->m_change_callback(false);
                            }
                        }
                    }
                }

                if (m_flags & Button::Flags::PopupButton)
                {
                    for (auto widget : parent()->children())
                    {
                        Button* b = dynamic_cast<Button*>(widget);
                        if (b != this && b && (b->flags() & PopupButton) && b->m_pushed)
                        {
                            b->m_pushed = false;
                            if (b->m_change_callback)
                                b->m_change_callback(false);
                        }
                    }
                }

                if (m_flags & ToggleButton)
                    m_pushed = !m_pushed;
                else
                    m_pushed = true;
            }
            else if (m_pushed)
            {
                if (this->contains(p) && m_pressed_callback)
                    m_pressed_callback();
                if (m_flags & NormalButton)
                    m_pushed = false;
            }
            if (pushedBackup != m_pushed && m_change_callback)
                m_change_callback(m_pushed);

            if (pushedBackup != m_pushed)
            {
                m_caption_texture.dirty = true;
                m_icon_texture.dirty = true;
            }
            return true;
        }
        return false;
    }

    void Button::set_text_color(const Color& text_color)
    {
        m_text_color = text_color;
        m_caption_texture.dirty = true;
        m_icon_texture.dirty = true;
    }

    Color Button::body_color()
    {
        Color result = m_theme->m_button_gradient_top_unfocused;
        if (m_background_color.a() != 0)
            result = m_background_color;

        if (m_pushed)
        {
            if (m_background_color.a() == 0)
                result = m_theme->m_button_gradient_top_pushed;
            else
            {
                result.b() *= 1.5;
                result.g() *= 1.5;
                result.r() *= 1.5;
            }
        }
        else if (m_mouse_focus && m_enabled)
        {
            if (m_background_color.a() == 0)
                result = m_theme->m_button_gradient_top_focused;
            else
            {
                result.b() *= 0.5;
                result.g() *= 0.5;
                result.r() *= 0.5;
            }
        }

        return result;
    }

    void Button::draw_body_temp(SDL3::SDL_Renderer* renderer)
    {
        Vector2i ap = absolute_position();
        SDL3::SDL_Color bodyclr = body_color().sdl_color();

        SDL3::SDL_FRect bodyRect{ float(ap.x + 1), float(ap.y + 1), float(this->width() - 2),
                                  float(this->height() - 2) };
        SDL3::SDL_SetRenderDrawColor(renderer, bodyclr.r, bodyclr.g, bodyclr.b, bodyclr.a);
        SDL3::SDL_RenderFillRect(renderer, &bodyRect);

        SDL3::SDL_FRect btnRect{ static_cast<float>(ap.x - 1), static_cast<float>(ap.y - 1),
                                 static_cast<float>(this->width() + 2),
                                 static_cast<float>(this->height() + 1) };
        SDL3::SDL_Color bl = (m_pushed ? m_theme->m_border_dark : m_theme->m_border_light)
                                 .sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
        SDL3::SDL_FRect blr{ static_cast<float>(ap.x),
                             static_cast<float>(ap.y + (m_pushed ? 1 : 2)),
                             static_cast<float>(this->width() - 1),
                             static_cast<float>(this->height() - 1 - (m_pushed ? 0 : 1)) };
        SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x + blr.w, blr.y);
        SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h - 1);

        SDL3::SDL_Color bd = (m_pushed ? m_theme->m_border_light : m_theme->m_border_dark)
                                 .sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
        SDL3::SDL_FRect bdr{ static_cast<float>(ap.x), static_cast<float>(ap.y + 1),
                             static_cast<float>(this->width() - 1),
                             static_cast<float>(this->height() - 2) };
        SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y + bdr.h, bdr.x + bdr.w, bdr.y + bdr.h);
        SDL3::SDL_RenderLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);

        bd = m_theme->m_border_dark.sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
        SDL3::SDL_RenderRect(renderer, &btnRect);
    }

    void Button::draw_body(const std::unique_ptr<rl::Renderer>& renderer)
    {
        i32 id{ 0 };
        id |= m_pushed ? 1 << 0 : 0;
        id |= m_mouse_focus ? 1 << 1 : 0;
        id |= m_enabled ? 1 << 2 : 0;

        std::shared_ptr<Button::AsyncTexture> texture{};
        for (auto&& tex : m_textures)
            if (tex->id == id)
            {
                texture = tex;
                break;
            }

        if (texture == nullptr)
        {
            texture = std::make_shared<Button::AsyncTexture>(id);
            texture->load(this);
            m_textures.push_back(texture);
        }

        runtime_assert(texture != nullptr, "Button: failed to create async texture");
        this->draw_texture(texture, renderer);
    }

    void Button::draw_body(SDL3::SDL_Renderer* renderer)
    {
        int id = (m_pushed ? 0x1 : 0) + (m_mouse_focus ? 0x2 : 0) + (m_enabled ? 0x4 : 0);
        auto atx = std::find_if(m_textures.begin(), m_textures.end(),
                                [id](const Button::AsyncTexturePtr& p) {
                                    return p->id == id;
                                });

        if (atx != m_textures.end())
            this->draw_texture(*atx, renderer);
        else
        {
            Button::AsyncTexturePtr new_texture = std::make_shared<Button::AsyncTexture>(id);
            new_texture->load(this);
            m_textures.push_back(new_texture);

            this->draw_texture(m_curr_texture, renderer);
        }
    }

    void Button::draw(const std::unique_ptr<rl::Renderer>& renderer)
    {
        Widget::draw(renderer);
        ds::point<f32> pos{ this->local_position() };
        this->draw_body(renderer);
    }

    void Button::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        Vector2i ap = absolute_position();
        this->draw_body(renderer);

        int font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
        if (m_caption_texture.dirty)
        {
            Color sdl_text_color = (m_text_color.a() == 0 ? m_theme->m_text_color : m_text_color);
            if (!m_enabled)
                sdl_text_color = m_theme->m_disabled_text_color;

            m_theme->get_texture_and_rect_utf8(renderer, m_caption_texture, 0, 0, m_caption.c_str(),
                                               "sans-bold", font_size, sdl_text_color);
        }

        Vector2f center(ap.x + width() * 0.5f, ap.y + height() * 0.5f);
        Vector2i textPos(center.x - m_caption_texture.w() * 0.5f,
                         center.y - m_caption_texture.h() * 0.5f - 1);

        int offset = m_pushed ? 2 : 0;

        if (m_icon)
        {
            float iw{ 0.0f };
            float ih{ static_cast<float>(font_size) };
            auto icon{ gui::utf8(m_icon) };

            if (m_icon_texture.dirty)
            {
                Color sdlTextColor = (m_text_color.a() == 0 ? m_theme->m_text_color : m_text_color);

                if (nvg_is_font_icon(m_icon))
                {
                    ih *= 1.5f;
                    m_theme->get_texture_and_rect_utf8(renderer, m_icon_texture, 0, 0, icon.data(),
                                                       "icons", ih, sdlTextColor);
                    iw = m_icon_texture.w();
                }
                else
                {
                    int w{};
                    int h{};
                    ih *= 0.9f;
                    iw = m_icon_texture.w() * ih / m_icon_texture.h();
                }
            }
            if (m_caption != "")
                iw += m_pos.y * 0.15f;

            Vector2i icon_pos = center.as<int>();
            icon_pos.y -= 1;

            if (m_icon_position == Button::IconPosition::LeftCentered)
            {
                icon_pos.x -= m_caption_texture.w() * 0.5f;
                icon_pos.x -= m_icon_texture.w() * 0.5f;
                textPos.x += m_icon_texture.w() * 0.5f;  // iw * 0.5f;
            }
            else if (m_icon_position == Button::IconPosition::RightCentered)
            {
                textPos.x -= iw * 0.5f;
                icon_pos.x += m_caption_texture.w() * 0.5f;
            }
            else if (m_icon_position == Button::IconPosition::Left)
            {
                icon_pos.x = get_absolute_left() + 8;
            }
            else if (m_icon_position == Button::IconPosition::Right)
            {
                icon_pos.x = get_absolute_left() + width() - iw - 8;
            }

            if (nvg_is_font_icon(m_icon))
            {
                auto&& pos = icon_pos.tofloat() + this->get_text_offset().tofloat() +
                             Vector2i(0, -m_icon_texture.h() * 0.5f + 1).tofloat();
                SDL3::SDL_FRect pos_rect{ pos.x, pos.y, 0.0f, 0.0f };
                SDL3::SDL_RenderTexture(renderer, m_icon_texture, &pos_rect, nullptr);
            }
            else
            {
                auto&& pos = icon_pos.tofloat() + get_text_offset().tofloat() +
                             Vector2i(0, -ih / 2).tofloat();
                SDL3::SDL_FRect pos_rect{ pos.x, pos.y, 0.0f, 0.0f };
                SDL3::SDL_RenderTexture(renderer, m_icon_texture, &pos_rect, nullptr);
            }
        }

        SDL_RenderCopy(renderer, m_caption_texture, textPos + get_text_offset());
    }

    Vector2i Button::get_text_offset() const
    {
        int offset = m_pushed ? 2 : 0;
        return Vector2i(offset, 1 + offset);
    }

    void Button::render_body_texture(NVGcontext*& ctx, int& realw, int& realh)
    {
        int ww = width();
        int hh = height();
        ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

        float pxRatio = 1.0f;
        realw = ww + 2;
        realh = hh + 2;
        nvgBeginFrame(ctx, realw, realh, pxRatio);

        NVGcolor gradTop = m_theme->m_button_gradient_top_unfocused.to_nvg_color();
        NVGcolor gradBot = m_theme->m_button_gradient_bot_unfocused.to_nvg_color();

        if (m_pushed)
        {
            gradTop = m_theme->m_button_gradient_top_pushed.to_nvg_color();
            gradBot = m_theme->m_button_gradient_bot_pushed.to_nvg_color();
        }
        else if (m_mouse_focus && m_enabled)
        {
            gradTop = m_theme->m_button_gradient_top_focused.to_nvg_color();
            gradBot = m_theme->m_button_gradient_bot_focused.to_nvg_color();
        }

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, 1, 1.0f, ww - 2, hh - 2, m_theme->m_button_corner_radius - 1);

        if (m_background_color.a() != 0)
        {
            Color rgb{ m_background_color.rgb() };
            rgb.set_alpha(1.f);

            nvgFillColor(ctx, rgb.to_nvg_color());
            nvgFill(ctx);

            if (m_pushed)
                gradTop.a = gradBot.a = 0.8f;
            else
            {
                float v = 1.0f - m_background_color.a();
                gradTop.a = gradBot.a = m_enabled ? v : v * .5f + .5f;
            }
        }

        NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop, gradBot);

        nvgFillPaint(ctx, bg);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgStrokeWidth(ctx, 1.0f);
        nvgRoundedRect(ctx, 0.5f, (m_pushed ? 0.5f : 1.5f), ww - 1,
                       hh - 1 - (m_pushed ? 0.0f : 1.0f), m_theme->m_button_corner_radius);
        nvgStrokeColor(ctx, m_theme->m_border_light.to_nvg_color());
        nvgStroke(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh - 2, m_theme->m_button_corner_radius);
        nvgStrokeColor(ctx, m_theme->m_border_dark.to_nvg_color());
        nvgStroke(ctx);

        nvgEndFrame(ctx);
    }

    void Button::draw_texture(std::shared_ptr<Button::AsyncTexture>& texture,
                              const std::unique_ptr<rl::Renderer>& renderer)
    {
        if (texture != nullptr)
            texture->perform(renderer);
    }

    void Button::draw_texture(std::shared_ptr<Button::AsyncTexture>& texture,
                              SDL3::SDL_Renderer* renderer)
    {
        if (texture)
        {
            texture->perform(renderer);

            if (texture->tex.tex)
            {
                auto&& pos = this->absolute_position().as<float>();
                SDL3::SDL_FRect pos_rect{ pos.x, pos.y, 0.0f, 0.0f };
                SDL3::SDL_RenderTexture(renderer, texture->tex, &pos_rect, nullptr);

                if (!m_curr_texture || texture->id != m_curr_texture->id)
                    m_curr_texture = texture;
            }
            else if (m_curr_texture)
            {
                auto&& pos = this->absolute_position().as<float>();
                SDL3::SDL_FRect pos_rect{ pos.x, pos.y, 0.0f, 0.0f };
                SDL3::SDL_RenderTexture(renderer, m_curr_texture->tex, &pos_rect, nullptr);
            }
            else
                this->draw_body_temp(renderer);
        }
        else
            draw_body_temp(renderer);
    }
}
