#include <array>
#include <thread>

#include "gui/checkbox.hpp"
#include "gui/entypo.hpp"
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
    struct CheckBox::AsyncTexture
    {
        int id;
        Texture tex;
        NVGcontext* ctx = nullptr;

        AsyncTexture(int _id)
            : id(_id){};

        void load(CheckBox* ptr, bool pushed, bool focused, bool enabled)
        {
            CheckBox* cb = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                Theme* theme = cb->theme();
                Color b = Color(0, 0, 0, 180);
                Color c = pushed ? Color(0, 100) : Color(0, 32);

                int ww = cb->width();
                int hh = cb->height();
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, ww + 2, hh + 2, pxRatio);

                NVGpaint bg = nvgBoxGradient(ctx, 1.5f, 1.5f, hh - 2.0f, hh - 2.0f, 3, 3,
                                             c.to_nvg_color(), b.to_nvg_color());

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, 1.0f, 1.0f, hh - 2.0f, hh - 2.0f, 3);
                nvgFillPaint(ctx, bg);
                nvgFill(ctx);

                nvgEndFrame(ctx);

                self->tex.rrect = { 0, 0, ww + 2, hh + 2 };
                self->ctx = ctx;
            });

            tgr.detach();
        }

        void perform(SDL3::SDL_Renderer* renderer)
        {
            if (!ctx)
                return;

            unsigned char* rgba = nvgReadPixelsRT(ctx);

            tex.tex = SDL_CreateTexture(renderer, SDL3::SDL_PIXELFORMAT_ABGR8888,
                                        SDL3::SDL_TEXTUREACCESS_STREAMING, tex.w(), tex.h());

            int pitch;
            uint8_t* pixels;
            int ok = SDL_LockTexture(tex.tex, nullptr, (void**)&pixels, &pitch);
            memcpy(pixels, rgba, sizeof(uint32_t) * tex.w() * tex.h());
            SDL_SetTextureBlendMode(tex.tex, SDL3::SDL_BLENDMODE_BLEND);
            SDL_UnlockTexture(tex.tex);

            nvgDeleteRT(ctx);
            ctx = nullptr;
        }
    };

    CheckBox::CheckBox(Widget* parent, const std::string& caption,
                       const std::function<void(bool)>& callback)
        : Widget(parent)
        , m_caption(caption)
        , m_pushed(false)
        , m_checked(false)
        , m_checkbox_callback(callback)
    {
        m_caption_texture.dirty = true;
        m_point_texture.dirty = true;
    }

    bool CheckBox::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        Widget::mouse_button_event(p, button, down, modifiers);
        if (!m_enabled)
            return false;

        if (button == SDL_BUTTON_LEFT)
        {
            if (down)
            {
                m_pushed = true;
            }
            else if (m_pushed)
            {
                if (contains(p))
                {
                    m_checked = !m_checked;
                    if (m_checkbox_callback)
                        m_checkbox_callback(m_checked);
                }
                m_pushed = false;
            }
            return true;
        }
        return false;
    }

    Vector2i CheckBox::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        if (m_fixed_size != Vector2i::zero())
            return m_fixed_size;

        int w, h;
        const_cast<CheckBox*>(this)->m_theme->get_text_bounds("sans", font_size(),
                                                              m_caption.c_str(), &w, &h);
        return Vector2i(w + 1.7f * font_size(), font_size() * 1.3f);
    }

    void CheckBox::draw_body(SDL3::SDL_Renderer* renderer)
    {
        int id = (m_pushed ? 0x1 : 0) + (m_mouse_focus ? 0x2 : 0) + (m_enabled ? 0x4 : 0);

        auto atx = std::find_if(m_textures.begin(), m_textures.end(),
                                [id](const CheckBox::AsyncTexturePtr& p) {
                                    return p->id == id;
                                });

        if (atx != m_textures.end())
            draw_texture(*atx, renderer);
        else
        {
            CheckBox::AsyncTexturePtr newtx = std::make_shared<CheckBox::AsyncTexture>(id);
            newtx->load(this, m_pushed, m_mouse_focus, m_enabled);
            m_textures.push_back(newtx);

            draw_texture(m_curr_texture, renderer);
        }
    }

    void CheckBox::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        if (m_caption_texture.dirty)
        {
            Color tColor = (m_enabled ? m_theme->m_text_color : m_theme->m_disabled_text_color);
            m_theme->get_texture_and_rect_utf8(renderer, m_caption_texture, 0, 0, m_caption.c_str(),
                                               "sans", font_size(), tColor);
            m_theme->get_texture_and_rect_utf8(renderer, m_point_texture, 0, 0,
                                               utf8(ENTYPO_ICON_CHECK).data(), "icons",
                                               1.8 * m_size.y, tColor);
        }

        auto ap = absolute_position();
        SDL_RenderCopy(
            renderer, m_caption_texture,
            ap + Vector2i(1.2f * m_size.y + 5, (m_size.y - m_caption_texture.h()) * 0.5f));

        draw_body(renderer);

        if (m_checked)
            SDL_RenderCopy(renderer, m_point_texture,
                           ap + Vector2i((m_size.y - m_point_texture.w()) * 0.5f + 1,
                                         (m_size.y - m_point_texture.h()) * 0.5f));
    }

    void CheckBox::draw_texture(AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer)
    {
        if (texture)
        {
            texture->perform(renderer);

            if (texture->tex.tex)
            {
                SDL_RenderCopy(renderer, texture->tex, absolute_position());

                if (!m_curr_texture || texture->id != m_curr_texture->id)
                    m_curr_texture = texture;
            }
            else if (m_curr_texture)
            {
                SDL_RenderCopy(renderer, m_curr_texture->tex, absolute_position());
            }
        }
    }
}
