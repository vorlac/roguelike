#include <mutex>
#include <thread>

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
                int ds = m_theme->mWindowDropShadowSize;

                Vector2i mPos(dx + ds, dy + ds);

                int realw = ww + 2 * ds + dx;  // with + 2*shadow + offset
                int realh = hh + 2 * ds + dy;
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, realw, realh, pxRatio);

                int cr = m_theme->mWindowCornerRadius;
                int headerH = m_theme->mWindowHeaderHeight;

                /* Draw window */
                nvgSave(ctx);
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, mPos.x, mPos.y, ww, hh, cr);

                nvgFillColor(
                    ctx, (mouseFocus ? m_theme->mWindowFillFocused : m_theme->mWindowFillUnfocused)
                             .toNvgColor());
                nvgFill(ctx);

                /* Draw a drop shadow */
                if (wnd->dropShadowEnabled())
                {
                    NVGpaint shadowPaint = nvgBoxGradient(ctx, mPos.x, mPos.y, ww, hh, cr * 2,
                                                          ds * 2, m_theme->mDropShadow.toNvgColor(),
                                                          m_theme->mTransparent.toNvgColor());

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
                    m_theme->mWindowHeaderGradientTop.toNvgColor(),
                    m_theme->mWindowHeaderGradientBot.toNvgColor());

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, mPos.x, mPos.y, ww, headerH, cr);

                nvgFillPaint(ctx, headerPaint);
                nvgFill(ctx);

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, mPos.x, mPos.y, ww, headerH, cr);
                nvgStrokeColor(ctx, m_theme->mWindowHeaderSepTop.toNvgColor());

                nvgSave(ctx);
                nvgIntersectScissor(ctx, mPos.x, mPos.y, ww, 0.5f);
                nvgStroke(ctx);
                nvgRestore(ctx);

                nvgBeginPath(ctx);
                nvgMoveTo(ctx, mPos.x + 0.5f, mPos.y + headerH - 1.5f);
                nvgLineTo(ctx, mPos.x + ww - 0.5f, mPos.y + headerH - 1.5);
                nvgStrokeColor(ctx, m_theme->mWindowHeaderSepBot.toNvgColor());
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
        , mTitle(title)
        , mButtonPanel(nullptr)
        , mModal(false)
        , mDrag(false)
    {
        _titleTex.dirty = true;
    }

    Vector2i Window::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        if (mButtonPanel)
            mButtonPanel->set_visible(false);
        Vector2i result = Widget::preferredSize(ctx);
        if (mButtonPanel)
            mButtonPanel->set_visible(true);

        int w, h;
        const_cast<Window*>(this)->m_theme->getTextBounds("sans-bold", 18.0, mTitle.c_str(), &w, &h);

        return result.cmax(Vector2i(w + 20, h));
    }

    Widget* Window::buttonPanel()
    {
        if (!mButtonPanel)
        {
            mButtonPanel = new Widget(this);
            mButtonPanel->set_layout(
                new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
        }
        return mButtonPanel;
    }

    void Window::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        if (!mButtonPanel)
        {
            Widget::perform_layout(ctx);
        }
        else
        {
            mButtonPanel->set_visible(false);
            Widget::perform_layout(ctx);
            for (auto w : mButtonPanel->children())
            {
                w->set_fixed_size({ 22, 22 });
                w->setFontSize(15);
            }
            mButtonPanel->set_visible(true);
            mButtonPanel->set_size({ width(), 22 });
            mButtonPanel->set_relative_position(
                { width() - (mButtonPanel->preferredSize(ctx).x + 5), 3 });
            mButtonPanel->perform_layout(ctx);
        }
    }

    bool Window::focusEvent(bool focused)
    {
        _titleTex.dirty = focused != m_focused;
        return Widget::focusEvent(focused);
    }

    void Window::drawBodyTemp(SDL3::SDL_Renderer* renderer)
    {
        int ds = m_theme->mWindowDropShadowSize;
        int cr = m_theme->mWindowCornerRadius;
        int hh = m_theme->mWindowHeaderHeight;

        Vector2i ap = absolute_position();
        SDL3::SDL_FRect rect{ ap.x, ap.y, m_size.x, m_size.y };

        /* Draw a drop shadow */
        SDL3::SDL_FRect shadowRect{ ap.x - ds, ap.y - ds, m_size.x + 2.0f * ds,
                                    m_size.y + 2.0f * ds };
        SDL3::SDL_Color shadowColor = m_theme->mDropShadow.toSdlColor();

        SDL3::SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, 32);
        SDL3::SDL_RenderFillRect(renderer, &shadowRect);

        /* Draw window */
        SDL3::SDL_Color color = (m_mouse_focus ? m_theme->mWindowFillFocused
                                               : m_theme->mWindowFillUnfocused)
                                    .toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL3::SDL_RenderFillRect(renderer, &rect);

        SDL3::SDL_FRect wndBdRect{ ap.x - 2, ap.y - 2, width() + 4, height() + 4 };
        SDL3::SDL_Color bd = m_theme->mBorderDark.toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
        SDL3::SDL_RenderRect(renderer, &wndBdRect);

        SDL3::SDL_Color headerColor = m_theme->mWindowHeaderGradientTop.toSdlColor();
        SDL3::SDL_FRect headerRect{ ap.x, ap.y, m_size.x, hh };

        SDL3::SDL_SetRenderDrawColor(renderer, headerColor.r, headerColor.g, headerColor.b,
                                     headerColor.a);
        SDL3::SDL_RenderFillRect(renderer, &headerRect);

        SDL3::SDL_Color headerBotColor = m_theme->mWindowHeaderSepBot.toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, headerBotColor.r, headerBotColor.g, headerBotColor.b,
                                     headerBotColor.a);
        SDL3::SDL_RenderLine(renderer, ap.x + 0.5f, ap.y + hh - 1.5f, ap.x + width() - 0.5f,
                             ap.y + hh - 1.5);
    }

    void Window::drawBody(SDL3::SDL_Renderer* renderer)
    {
        int id = (m_mouse_focus ? 0x1 : 0);

        auto atx = std::find_if(m_window_txs.begin(), m_window_txs.end(),
                                [id](const Window::AsyncTexturePtr& p) {
                                    return p->id == id;
                                });

        if (atx != m_window_txs.end())
            drawTexture(*atx, renderer);
        else
        {
            Window::AsyncTexturePtr newtx = std::make_shared<Window::AsyncTexture>(id);
            newtx->load(this, 0, 0, m_mouse_focus);
            m_window_txs.push_back(newtx);

            drawTexture(current_texture_, renderer);
        }
    }

    void Window::draw(SDL3::SDL_Renderer* renderer)
    {
        drawBody(renderer);

        if (_titleTex.dirty)
        {
            Color titleTextColor = (m_focused ? m_theme->mWindowTitleFocused
                                              : m_theme->mWindowTitleUnfocused);
            m_theme->getTexAndRectUtf8(renderer, _titleTex, 0, 0, mTitle.c_str(), "sans-bold", 18,
                                       titleTextColor);
        }

        if (!mTitle.empty() && _titleTex.tex)
        {
            int headerH = m_theme->mWindowHeaderHeight;
            SDL_RenderCopy(
                renderer, _titleTex,
                m_pos + Vector2i((m_size.x - _titleTex.w()) / 2, (headerH - _titleTex.h()) / 2));
        }

        Widget::draw(renderer);
    }

    void Window::dispose()
    {
        Widget* widget = this;
        while (widget->parent())
            widget = widget->parent();
        ((Screen*)widget)->dispose_window(this);
    }

    void Window::center()
    {
        Widget* widget = this;
        while (widget->parent())
            widget = widget->parent();
        ((Screen*)widget)->center_window(this);
    }

    bool Window::mouseDragEvent(const Vector2i&, const Vector2i& rel, int button,
                                int /* modifiers */)
    {
        if (!mDraggable)
            return false;
        if (mDrag && (button & (1 << SDL_BUTTON_LEFT)) != 0)
        {
            m_pos += rel;
            m_pos = m_pos.cmax({ 0, 0 });
            m_pos = m_pos.cmin(parent()->size() - m_size);
            return true;
        }
        return false;
    }

    bool Window::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
    {
        if (Widget::mouseButtonEvent(p, button, down, modifiers))
            return true;
        if (button == SDL_BUTTON_LEFT)
        {
            mDrag = down && (p.y - m_pos.y) < m_theme->mWindowHeaderHeight;
            return true;
        }
        return false;
    }

    bool Window::scrollEvent(const Vector2i& p, const Vector2f& rel)
    {
        Widget::scrollEvent(p, rel);
        return true;
    }

    void Window::refreshRelativePlacement()
    {
        // Overridden in Popup
    }

    void Window::drawTexture(AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer)
    {
        if (texture)
        {
            texture->perform(renderer);

            if (texture->tex.tex)
            {
                SDL_RenderCopy(renderer, texture->tex, absolute_position());

                if (!current_texture_ || texture->id != current_texture_->id)
                    current_texture_ = texture;
            }
            else if (current_texture_)
            {
                SDL_RenderCopy(renderer, current_texture_->tex, absolute_position());
            }
            else
                drawBodyTemp(renderer);
        }
        else
            drawBodyTemp(renderer);
    }

}
