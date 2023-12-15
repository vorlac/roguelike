/*
    sdlgui/popup.cpp -- Simple popup widget which is attached to another given
    window (can be nested)

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <thread>

#include "gui/popup.hpp"
#include "gui/theme.hpp"
#include "nanovg.h"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"

namespace rl::gui {

    struct Popup::AsyncTexture
    {
        int id;
        Texture tex;
        NVGcontext* ctx = nullptr;

        AsyncTexture(int _id)
            : id(_id){};

        void load(Popup* ptr, int dx)
        {
            Popup* pp = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                std::lock_guard<std::mutex> guard(pp->theme()->loadMutex);

                NVGcontext* ctx = nullptr;
                int realw, realh;
                pp->rendereBodyTexture(ctx, realw, realh, dx);
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

    Popup::Popup(Widget* parent, Window* parentWindow)
        : Window(parent, "")
        , mParentWindow(parentWindow)
        , mAnchorPos(Vector2i::Zero())
        , mAnchorHeight(30)
    {
    }

    void Popup::rendereBodyTexture(NVGcontext*& ctx, int& realw, int& realh, int dx)
    {
        int ww = width();
        int hh = height();
        int ds = m_theme->mWindowDropShadowSize;
        int dy = 0;

        Vector2i offset(dx + ds, dy + ds);

        realw = ww + 2 * ds + dx;  // with + 2*shadow + offset
        realh = hh + 2 * ds + dy;

        ctx = nvgCreateRT(NVG_DEBUG, realw, realh, 0);

        float pxRatio = 1.0f;
        nvgBeginFrame(ctx, realw, realh, pxRatio);

        int cr = m_theme->mWindowCornerRadius;

        /* Draw a drop shadow */
        NVGpaint shadowPaint = nvgBoxGradient(ctx, offset.x, offset.y, ww, hh, cr * 2, ds * 2,
                                              m_theme->mDropShadow.toNvgColor(),
                                              m_theme->mTransparent.toNvgColor());

        nvgBeginPath(ctx);
        // nvgRect(ctx, offset.x - ds, offset.y - ds, ww + 2 * ds, hh + 2 * ds);
        nvgRoundedRect(ctx, offset.x - ds, offset.y - ds, ww + 2 * ds, hh + 2 * ds, cr);
        // nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);

        /* Draw window */
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, offset.x, offset.y, ww, hh, cr);

        Vector2i base = Vector2i(offset.x + 0, offset.y + anchorHeight());
        int sign = -1;

        nvgMoveTo(ctx, base.x + 15 * sign, base.y);
        nvgLineTo(ctx, base.x, base.y - 15);
        nvgLineTo(ctx, base.x, base.y + 15);

        nvgFillColor(ctx, m_theme->mWindowPopup.toNvgColor());
        nvgFill(ctx);
        nvgEndFrame(ctx);
    }

    void Popup::performLayout(SDL3::SDL_Renderer* ctx)
    {
        if (mLayout || mChildren.size() != 1)
        {
            Widget::performLayout(ctx);
        }
        else
        {
            mChildren[0]->setPosition(Vector2i::Zero());
            mChildren[0]->setSize(mSize);
            mChildren[0]->performLayout(ctx);
        }
    }

    void Popup::refreshRelativePlacement()
    {
        mParentWindow->refreshRelativePlacement();
        m_visible &= mParentWindow->visibleRecursive();

        Widget* widget = this;
        while (widget->parent() != nullptr)
            widget = widget->parent();
        Screen* screen = (Screen*)widget;
        Vector2i screenSize = screen->size();

        _pos = mParentWindow->position() + mAnchorPos - Vector2i(0, mAnchorHeight);
        _pos = Vector2i(_pos.x, std::min(_pos.y, screen->size().y - mSize.y));
    }

    void Popup::drawBodyTemp(SDL3::SDL_Renderer* renderer)
    {
        int ds = m_theme->mWindowDropShadowSize;
        int cr = m_theme->mWindowCornerRadius;

        /* Draw a drop shadow */
        SDL3::SDL_Color sh = m_theme->mDropShadow.toSdlColor();
        SDL3::SDL_FRect shRect{
            static_cast<float>(_pos.x - ds),
            static_cast<float>(_pos.y - ds),
            static_cast<float>(mSize.x + 2 * ds),
            static_cast<float>(mSize.y + 2 * ds),
        };
        SDL3::SDL_SetRenderDrawColor(renderer, sh.r, sh.g, sh.b, 64);
        SDL3::SDL_RenderFillRect(renderer, &shRect);

        SDL3::SDL_Color bg = m_theme->mWindowPopup.toSdlColor();
        SDL3::SDL_FRect bgRect{
            static_cast<float>(_pos.x),
            static_cast<float>(_pos.y),
            static_cast<float>(mSize.x),
            static_cast<float>(mSize.y),
        };

        SDL3::SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
        SDL3::SDL_RenderFillRect(renderer, &bgRect);

        SDL3::SDL_Color br = m_theme->mBorderDark.toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, br.r, br.g, br.b, br.a);

        SDL3::SDL_Rect brr{ _pos.x - 1, _pos.y - 1, width() + 2, height() + 2 };
        SDL3::SDL_RenderLine(renderer, brr.x, brr.y, brr.x + brr.w, brr.y);
        SDL3::SDL_RenderLine(renderer, brr.x + brr.w, brr.y, brr.x + brr.w, brr.y + brr.h);
        SDL3::SDL_RenderLine(renderer, brr.x, brr.y + brr.h, brr.x + brr.w, brr.y + brr.h);
        SDL3::SDL_RenderLine(renderer, brr.x, brr.y, brr.x, brr.y + brr.h);

        // Draw window anchor
        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
        for (int i = 0; i < 15; i++)
        {
            SDL3::SDL_RenderLine(renderer, _pos.x - 15 + i, _pos.y + mAnchorHeight - i,
                                 _pos.x - 15 + i, _pos.y + mAnchorHeight + i);
        }
    }

    void Popup::drawBody(SDL3::SDL_Renderer* renderer)
    {
        int id = 1;

        auto atx = std::find_if(m_popup_txs.begin(), m_popup_txs.end(),
                                [id](Popup::AsyncTexturePtr p) {
                                    return p->id == id;
                                });

        if (atx != m_popup_txs.end())
        {
            (*atx)->perform(renderer);

            if ((*atx)->tex.tex)
            {
                auto&& pos = getOverrideBodyPos().tofloat();
                SDL3::SDL_FRect rect{ pos.x, pos.y, 0.0f, 0.0f };
                SDL_RenderTexture(renderer, (*atx)->tex.tex, &rect, nullptr);
            }
            else
                drawBodyTemp(renderer);
        }
        else
        {
            Popup::AsyncTexturePtr newtx = std::make_shared<Popup::AsyncTexture>(id);
            newtx->load(this, _anchorDx);
            m_popup_txs.push_back(newtx);
        }
    }

    Vector2i Popup::getOverrideBodyPos()
    {
        Vector2i ap = absolutePosition();
        int ds = m_theme->mWindowDropShadowSize;
        return ap - Vector2i(_anchorDx + ds, ds);
    }

    void Popup::draw(SDL3::SDL_Renderer* renderer)
    {
        refreshRelativePlacement();

        if (!m_visible)
            return;

        drawBody(renderer);

        Widget::draw(renderer);
    }

}
