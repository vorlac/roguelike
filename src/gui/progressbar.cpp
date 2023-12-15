#include <thread>

#include "gui/nanovg.h"
#include "gui/progressbar.hpp"
#include "gui/theme.hpp"

#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "gui/nanovg_rt.h"

namespace rl::gui {
    struct ProgressBar::AsyncTexture
    {
        Texture tex;
        NVGcontext* ctx = nullptr;
        float value = -1;
        bool busy = false;

        void load_body(ProgressBar* ptr)
        {
            ProgressBar* pbar = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                Theme* mTheme = pbar->theme();
                std::lock_guard<std::mutex> guard(mTheme->loadMutex);

                int ww = pbar->width();
                int hh = pbar->height();
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, ww + 2, hh + 2, pxRatio);

                NVGpaint paint = nvgBoxGradient(ctx, 1, 1, ww - 2, hh, 3, 4,
                                                Color(0, 32).toNvgColor(),
                                                Color(0, 92).toNvgColor());
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, 0, 0, ww, hh, 3);
                nvgFillPaint(ctx, paint);
                nvgFill(ctx);

                nvgEndFrame(ctx);
                self->tex.rrect = { 0, 0, ww + 2, hh + 2 };
                self->ctx = ctx;
            });

            tgr.detach();
        }

        void load_bar(ProgressBar* ptr)
        {
            ProgressBar* pbar = ptr;
            AsyncTexture* self = this;

            if (busy)
                return;

            busy = true;

            std::thread tgr([=]() {
                Theme* mTheme = pbar->theme();
                std::lock_guard<std::mutex> guard(mTheme->loadMutex);

                int ww = pbar->width();
                int hh = pbar->height();
                NVGcontext* ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

                float pxRatio = 1.0f;
                nvgBeginFrame(ctx, ww + 2, hh + 2, pxRatio);

                float value = std::min(std::max(0.0f, pbar->value()), 1.0f);
                int barPos = (int)std::round((ww - 2) * value);

                NVGpaint paint = nvgBoxGradient(ctx, 0, 0, barPos + 1.5f, hh - 1, 3, 4,
                                                Color(220, 100).toNvgColor(),
                                                Color(128, 100).toNvgColor());

                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, 1, 1, barPos, hh - 2, 3);
                nvgFillPaint(ctx, paint);
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

            if (tex.tex)
            {
                int w, h;
                SDL_QueryTexture(tex.tex, nullptr, nullptr, &w, &h);
                if (w != tex.w() || h != tex.h())
                    SDL_DestroyTexture(tex.tex);
            }

            if (!tex.tex)
                tex.tex = SDL3::SDL_CreateTexture(renderer, SDL3::SDL_PIXELFORMAT_ABGR8888,
                                                  SDL3::SDL_TEXTUREACCESS_STREAMING, tex.w(),
                                                  tex.h());

            int pitch;
            uint8_t* pixels;
            int ok = SDL_LockTexture(tex.tex, nullptr, (void**)&pixels, &pitch);
            memcpy(pixels, rgba, sizeof(uint32_t) * tex.w() * tex.h());
            SDL_SetTextureBlendMode(tex.tex, SDL3::SDL_BLENDMODE_BLEND);
            SDL_UnlockTexture(tex.tex);

            nvgDeleteRT(ctx);
            ctx = nullptr;
            busy = false;
        }
    };

    ProgressBar::ProgressBar(Widget* parent)
        : Widget(parent)
        , mValue(0.0f)
    {
    }

    void ProgressBar::setValue(float value)
    {
        mValue = value;
    }

    Vector2i ProgressBar::preferredSize(SDL3::SDL_Renderer*) const
    {
        return Vector2i(70, 12);
    }

    void ProgressBar::drawBody(SDL3::SDL_Renderer* renderer)
    {
        if (!_body)
        {
            _body = std::make_shared<AsyncTexture>();
            _body->load_body(this);
        }

        if (_body)
        {
            Vector2i ap = absolute_position();
            _body->perform(renderer);
            SDL_RenderCopy(renderer, _body->tex, ap);
        }
    }

    void ProgressBar::drawBar(SDL3::SDL_Renderer* renderer)
    {
        if (!_bar)
            _bar = std::make_shared<AsyncTexture>();

        if (mValue != _bar->value)
            _bar->load_bar(this);

        if (_bar)
        {
            Vector2i ap = absolute_position();
            _bar->perform(renderer);
            SDL_RenderCopy(renderer, _bar->tex, ap);
        }
    }

    void ProgressBar::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        drawBody(renderer);
        drawBar(renderer);
    }
}
