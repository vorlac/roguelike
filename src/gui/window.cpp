#include <mutex>
#include <thread>

#include "core/assert.hpp"
#include "gui/layout.hpp"
#include "gui/nanovg.h"
#include "gui/screen.hpp"
#include "gui/theme.hpp"
#include "gui/window.hpp"
#include "sdl/defs.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

#pragma warning(disable : 4838)

namespace rl::gui {
    struct Window::AsyncTexture
    {
        int id;
        Texture tex;
        NVGcontext* ctx = nullptr;

        bool ready() const
        {
            return tex.tex != nullptr;
        }

        AsyncTexture(int _id)
            : id(_id){};

        void load(Window* ptr, int dx, int dy, bool mouseFocus)
        {
            Window* wnd = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                Theme* m_theme = wnd->theme();

                int ww = wnd->width();
                int hh = wnd->height();
                int ds = m_theme->m_window_drop_shadow_size;

                Vector2i mPos(dx + ds, dy + ds);

                int realw = ww + 2 * ds + dx;  // with + 2*shadow + offset
                int realh = hh + 2 * ds + dy;
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, realw, realh, pxRatio);

                int cr = m_theme->m_window_corner_radius;
                int headerH = m_theme->m_window_header_height;

                /* Draw window */
                nvgSave(ctx);
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, mPos.x, mPos.y, ww, hh, cr);

                nvgFillColor(ctx, (mouseFocus ? m_theme->m_window_fill_focused
                                              : m_theme->m_window_fill_unfocused)
                                      .to_nvg_color());
                nvgFill(ctx);

                /* Draw a drop shadow */
                if (wnd->drop_shadow_enabled())
                {
                    NVGpaint shadowPaint = nvgBoxGradient(ctx, mPos.x, mPos.y, ww, hh, cr * 2,
                                                          ds * 2,
                                                          m_theme->m_drop_shadow.to_nvg_color(),
                                                          m_theme->m_transparent.to_nvg_color());

                    nvgSave(ctx);
                    nvgResetScissor(ctx);
                    nvgBeginPath(ctx);
                    nvgRect(ctx, mPos.x - ds, mPos.y - ds, ww + 2 * ds, hh + 2 * ds);
                    nvgRoundedRect(ctx, mPos.x, mPos.y, ww, hh, cr);
                    nvgPathWinding(ctx, NVG_HOLE);
                    nvgFillPaint(ctx, shadowPaint);
                    nvgFill(ctx);
                    nvgRestore(ctx);
                }

                /* Draw header */
                NVGpaint headerPaint = nvgLinearGradient(
                    ctx, mPos.x, mPos.y, mPos.x, mPos.y + headerH,
                    m_theme->m_window_header_gradient_top.to_nvg_color(),
                    m_theme->m_window_header_gradient_bot.to_nvg_color());

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, mPos.x, mPos.y, ww, headerH, cr);

                nvgFillPaint(ctx, headerPaint);
                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, mPos.x, mPos.y, ww, headerH, cr);
                nvgStrokeColor(ctx, m_theme->m_window_header_sep_top.to_nvg_color());

                nvgSave(ctx);
                nvgIntersectScissor(ctx, mPos.x, mPos.y, ww, 0.5f);
                nvgStroke(ctx);
                nvgRestore(ctx);

                nvgBeginPath(ctx);
                nvgMoveTo(ctx, mPos.x + 0.5f, mPos.y + headerH - 1.5f);
                nvgLineTo(ctx, mPos.x + ww - 0.5f, mPos.y + headerH - 1.5);
                nvgStrokeColor(ctx, m_theme->m_window_header_sep_bot.to_nvg_color());
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

    Window::Window(Widget* parent, const std::string& title)
        : Widget(parent)
        , m_title(title)
        , m_button_panel(nullptr)
        , m_modal(false)
        , m_drag(false)
    {
        m_title_texture.dirty = true;
    }

    Vector2i Window::preferred_size(SDL3::SDL_Renderer* ctx) const
    {
        if (m_button_panel)
            m_button_panel->set_visible(false);
        Vector2i result = Widget::preferred_size(ctx);
        if (m_button_panel)
            m_button_panel->set_visible(true);

        int w, h;
        const_cast<Window*>(this)->m_theme->get_text_bounds("sans-bold", 18.0, m_title.c_str(), &w,
                                                            &h);

        return result.cmax(Vector2i(w + 20, h));
    }

    Widget* Window::buttonPanel()
    {
        if (!m_button_panel)
        {
            m_button_panel = new Widget(this);
            m_button_panel->set_layout(
                new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
        }
        return m_button_panel;
    }

    void Window::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        if (!m_button_panel)
        {
            Widget::perform_layout(ctx);
        }
        else
        {
            m_button_panel->set_visible(false);
            Widget::perform_layout(ctx);
            for (auto w : m_button_panel->children())
            {
                w->set_fixed_size({ 22, 22 });
                w->set_font_size(15);
            }
            m_button_panel->set_visible(true);
            m_button_panel->set_size({ this->width(), 22 });
            m_button_panel->set_relative_position(
                { this->width() - (m_button_panel->preferred_size(ctx).x + 5), 3 });
            m_button_panel->perform_layout(ctx);
        }
    }

    bool Window::focus_event(bool focused)
    {
        m_title_texture.dirty = focused != m_focused;
        return Widget::focus_event(focused);
    }

    void Window::draw_body_temp(SDL3::SDL_Renderer* renderer)
    {
        int ds = m_theme->m_window_drop_shadow_size;
        int cr = m_theme->m_window_corner_radius;
        int hh = m_theme->m_window_header_height;

        Vector2i ap = absolute_position();
        SDL3::SDL_FRect rect{ ap.x, ap.y, m_size.x, m_size.y };

        /* Draw a drop shadow */
        SDL3::SDL_FRect shadowRect{ ap.x - ds, ap.y - ds, m_size.x + 2.0f * ds,
                                    m_size.y + 2.0f * ds };
        SDL3::SDL_Color shadowColor = m_theme->m_drop_shadow.sdl_color();

        SDL3::SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, 32);
        SDL3::SDL_RenderFillRect(renderer, &shadowRect);

        /* Draw window */
        SDL3::SDL_Color color = (m_mouse_focus ? m_theme->m_window_fill_focused
                                               : m_theme->m_window_fill_unfocused)
                                    .sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL3::SDL_RenderFillRect(renderer, &rect);

        SDL3::SDL_FRect wndBdRect{ ap.x - 2, ap.y - 2, this->width() + 4, this->height() + 4 };
        SDL3::SDL_Color bd = m_theme->m_border_dark.sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
        SDL3::SDL_RenderRect(renderer, &wndBdRect);

        SDL3::SDL_Color headerColor = m_theme->m_window_header_gradient_top.sdl_color();
        SDL3::SDL_FRect headerRect{ ap.x, ap.y, m_size.x, hh };

        SDL3::SDL_SetRenderDrawColor(renderer, headerColor.r, headerColor.g, headerColor.b,
                                     headerColor.a);
        SDL3::SDL_RenderFillRect(renderer, &headerRect);

        SDL3::SDL_Color headerBotColor = m_theme->m_window_header_sep_bot.sdl_color();
        SDL3::SDL_SetRenderDrawColor(renderer, headerBotColor.r, headerBotColor.g, headerBotColor.b,
                                     headerBotColor.a);
        SDL3::SDL_RenderLine(renderer, ap.x + 0.5f, ap.y + hh - 1.5f, ap.x + this->width() - 0.5f,
                             ap.y + hh - 1.5);
    }

    void Window::draw_body(SDL3::SDL_Renderer* renderer)
    {
        int id = (m_mouse_focus ? 0x1 : 0);

        auto atx = std::find_if(m_window_textures.begin(), m_window_textures.end(),
                                [id](const Window::AsyncTexturePtr& p) {
                                    return p->id == id;
                                });

        if (atx != m_window_textures.end())
            draw_texture(*atx, renderer);
        else
        {
            Window::AsyncTexturePtr newtx = std::make_shared<Window::AsyncTexture>(id);
            newtx->load(this, 0, 0, m_mouse_focus);
            m_window_textures.push_back(newtx);

            draw_texture(m_curr_texture, renderer);
        }
    }

    void Window::draw(SDL3::SDL_Renderer* renderer)
    {
        this->draw_body(renderer);

        if (m_title_texture.dirty)
        {
            Color titleTextColor = (m_focused ? m_theme->m_window_title_focused
                                              : m_theme->m_window_fill_unfocused);
            m_theme->get_texture_and_rect_utf8(renderer, m_title_texture, 0, 0, m_title.c_str(),
                                               "sans-bold", 18, titleTextColor);
        }

        if (!m_title.empty() && m_title_texture.tex)
        {
            i32 header_h{ m_theme->m_window_header_height };
            auto&& pos = m_pos + Vector2i((m_size.x - m_title_texture.w()) / 2,
                                          (header_h - m_title_texture.h()) / 2);
            SDL3::SDL_FRect rect{ pos.x, pos.y, 0.0f, 0.0f };
            i32 result = SDL3::SDL_RenderTexture(renderer, m_title_texture, &rect, nullptr);
            sdl_assert(result == 0, "Render texture failed: {}", result);
        }

        this->draw(renderer);
    }

    void Window::dispose()
    {
        Widget* widget{ this };
        while (widget->parent() != nullptr)
            widget = widget->parent();

        Screen* screen{ static_cast<Screen*>(widget) };
        runtime_assert(screen != nullptr, "Invalid screen widget");
        screen->dispose_window(this);
    }

    void Window::center()
    {
        Widget* widget{ this };
        while (widget->parent() != nullptr)
            widget = widget->parent();

        static_cast<Screen*>(widget)->center_window(this);
    }

    bool Window::mouse_drag_event(const Vector2i&, const Vector2i& rel, i32 button,
                                  i32 /* modifiers */)
    {
        if (!m_draggable)
            return false;

        if (m_drag && (button & (1 << SDL_BUTTON_LEFT)) != 0)
        {
            m_pos += rel;
            m_pos = m_pos.cmax({ 0, 0 });
            m_pos = m_pos.cmin(parent()->size() - m_size);
            return true;
        }
        return false;
    }

    bool Window::mouse_button_event(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (Widget::mouse_button_event(p, button, down, modifiers))
            return true;

        if (button == SDL_BUTTON_LEFT)
        {
            m_drag = down && (p.y - m_pos.y) < m_theme->m_window_header_height;
            return true;
        }
        return false;
    }

    bool Window::scroll_event(const Vector2i& p, const Vector2f& rel)
    {
        Widget::scroll_event(p, rel);
        return true;
    }

    void Window::refresh_relative_placement()
    {
        // Overridden in Popup
    }

    void Window::draw_texture(AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer)
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
            else
                this->draw_body_temp(renderer);
        }
        else
            this->draw_body_temp(renderer);
    }

}
