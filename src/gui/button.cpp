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
        int id;
        Texture tex;
        NVGcontext* ctx = nullptr;

        AsyncTexture(int _id)
            : id(_id){};

        void load(Button* ptr)
        {
            Button* button = ptr;
            AsyncTexture* self = this;
            std::thread tgr([=]() {
                std::lock_guard<std::mutex> guard(button->theme()->loadMutex);

                NVGcontext* ctx = nullptr;
                int realw, realh;
                button->renderBodyTexture(ctx, realw, realh);
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

    Button::Button(Widget* parent, const std::string& caption, int icon)
        : Widget(parent)
        , mCaption(caption)
        , mIcon(icon)
        , mIconPosition(IconPosition::LeftCentered)
        , mPushed(false)
        , mFlags(NormalButton)
        , mBackgroundColor(Color(0, 0))
        , mTextColor(Color(0, 0))
    {
        _captionTex.dirty = true;
        _iconTex.dirty = true;
    }

    Vector2i Button::preferredSize(SDL3::SDL_Renderer* ctx) const
    {
        int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
        float tw = const_cast<Button*>(this)->mTheme->getTextWidth("sans-bold", fontSize,
                                                                   mCaption.c_str());
        float iw = 0.0f, ih = fontSize;

        if (mIcon)
        {
            if (nvgIsFontIcon(mIcon))
            {
                ih *= 1.5f;
                iw = const_cast<Button*>(this)->mTheme->getUtf8Width("icons", ih,
                                                                     utf8(mIcon).data()) +
                     mSize.y * 0.15f;
            }
            else
            {
                int w, h;
                ih *= 0.9f;
                SDL3::SDL_QueryTexture((SDL3::SDL_Texture*)mIcon, nullptr, nullptr, &w, &h);
                iw = w * ih / h;
            }
        }
        return Vector2i((int)(tw + iw) + 20, fontSize + 10);
    }

    bool Button::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers)
    {
        Widget::mouseButtonEvent(p, button, down, modifiers);
        /* Temporarily increase the reference count of the button in case the
           button causes the parent window to be destructed */
        ref<Button> self = this;

        if (button == SDL_BUTTON_LEFT && mEnabled)
        {
            bool pushedBackup = mPushed;
            if (down)
            {
                if (mFlags & RadioButton)
                {
                    if (mButtonGroup.empty())
                    {
                        for (auto widget : parent()->children())
                        {
                            Button* b = dynamic_cast<Button*>(widget);
                            if (b != this && b && (b->flags() & RadioButton) && b->mPushed)
                            {
                                b->mPushed = false;
                                if (b->mChangeCallback)
                                    b->mChangeCallback(false);
                            }
                        }
                    }
                    else
                    {
                        for (auto b : mButtonGroup)
                        {
                            if (b != this && (b->flags() & RadioButton) && b->mPushed)
                            {
                                b->mPushed = false;
                                if (b->mChangeCallback)
                                    b->mChangeCallback(false);
                            }
                        }
                    }
                }

                if (mFlags & PopupButton)
                {
                    for (auto widget : parent()->children())
                    {
                        Button* b = dynamic_cast<Button*>(widget);
                        if (b != this && b && (b->flags() & PopupButton) && b->mPushed)
                        {
                            b->mPushed = false;
                            if (b->mChangeCallback)
                                b->mChangeCallback(false);
                        }
                    }
                }

                if (mFlags & ToggleButton)
                    mPushed = !mPushed;
                else
                    mPushed = true;
            }
            else if (mPushed)
            {
                if (contains(p) && mCallback)
                    mCallback();
                if (mFlags & NormalButton)
                    mPushed = false;
            }
            if (pushedBackup != mPushed && mChangeCallback)
                mChangeCallback(mPushed);

            if (pushedBackup != mPushed)
            {
                _captionTex.dirty = true;
                _iconTex.dirty = true;
            }
            return true;
        }
        return false;
    }

    void Button::setTextColor(const Color& textColor)
    {
        mTextColor = textColor;
        _captionTex.dirty = true;
        _iconTex.dirty = true;
    }

    Color Button::bodyColor()
    {
        Color result = mTheme->mButtonGradientTopUnfocused;
        if (mBackgroundColor.a() != 0)
            result = mBackgroundColor;

        if (mPushed)
        {
            if (mBackgroundColor.a() != 0)
            {
                result.b() *= 1.5;
                result.g() *= 1.5;
                result.r() *= 1.5;
            }
            else
                result = mTheme->mButtonGradientTopPushed;
        }
        else if (mMouseFocus && mEnabled)
        {
            if (mBackgroundColor.a() != 0)
            {
                result.b() *= 0.5;
                result.g() *= 0.5;
                result.r() *= 0.5;
            }
            else
                result = mTheme->mButtonGradientTopFocused;
        }

        return result;
    }

    void Button::drawBodyTemp(SDL3::SDL_Renderer* renderer)
    {
        Vector2i ap = absolutePosition();
        SDL3::SDL_Color bodyclr = bodyColor().toSdlColor();

        SDL3::SDL_FRect bodyRect{ float(ap.x + 1), float(ap.y + 1), float(width() - 2),
                                  float(height() - 2) };
        SDL3::SDL_SetRenderDrawColor(renderer, bodyclr.r, bodyclr.g, bodyclr.b, bodyclr.a);
        SDL3::SDL_RenderFillRect(renderer, &bodyRect);

        SDL3::SDL_FRect btnRect{ static_cast<float>(ap.x - 1), static_cast<float>(ap.y - 1),
                                 static_cast<float>(width() + 2),
                                 static_cast<float>(height() + 1) };
        SDL3::SDL_Color bl = (mPushed ? mTheme->mBorderDark : mTheme->mBorderLight).toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
        SDL3::SDL_FRect blr{ static_cast<float>(ap.x), static_cast<float>(ap.y + (mPushed ? 1 : 2)),
                             static_cast<float>(width() - 1),
                             static_cast<float>(height() - 1 - (mPushed ? 0 : 1)) };
        SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x + blr.w, blr.y);
        SDL3::SDL_RenderLine(renderer, blr.x, blr.y, blr.x, blr.y + blr.h - 1);

        SDL3::SDL_Color bd = (mPushed ? mTheme->mBorderLight : mTheme->mBorderDark).toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
        SDL3::SDL_FRect bdr{ static_cast<float>(ap.x), static_cast<float>(ap.y + 1),
                             static_cast<float>(width() - 1), static_cast<float>(height() - 2) };
        SDL3::SDL_RenderLine(renderer, bdr.x, bdr.y + bdr.h, bdr.x + bdr.w, bdr.y + bdr.h);
        SDL3::SDL_RenderLine(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);

        bd = mTheme->mBorderDark.toSdlColor();
        SDL3::SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
        SDL3::SDL_RenderRect(renderer, &btnRect);
    }

    void Button::drawBody(SDL3::SDL_Renderer* renderer)
    {
        int id = (mPushed ? 0x1 : 0) + (mMouseFocus ? 0x2 : 0) + (mEnabled ? 0x4 : 0);

        auto atx = std::find_if(_txs.begin(), _txs.end(), [id](const AsyncTexturePtr& p) {
            return p->id == id;
        });

        if (atx != _txs.end())
            drawTexture(*atx, renderer);
        else
        {
            AsyncTexturePtr new_texture = std::make_shared<AsyncTexture>(id);
            new_texture->load(this);
            _txs.push_back(new_texture);

            drawTexture(current_texture_, renderer);
        }
    }

    void Button::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        Vector2i ap = absolutePosition();
        drawBody(renderer);

        int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
        if (_captionTex.dirty)
        {
            Color sdlTextColor = (mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor);
            if (!mEnabled)
                sdlTextColor = mTheme->mDisabledTextColor;

            mTheme->getTexAndRectUtf8(renderer, _captionTex, 0, 0, mCaption.c_str(), "sans-bold",
                                      fontSize, sdlTextColor);
        }

        Vector2f center(ap.x + width() * 0.5f, ap.y + height() * 0.5f);
        Vector2i textPos(center.x - _captionTex.w() * 0.5f, center.y - _captionTex.h() * 0.5f - 1);

        int offset = mPushed ? 2 : 0;

        if (mIcon)
        {
            float iw = 0, ih = fontSize;
            auto icon = utf8(mIcon);

            if (_iconTex.dirty)
            {
                Color sdlTextColor = (mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor);

                if (nvgIsFontIcon(mIcon))
                {
                    ih *= 1.5f;
                    mTheme->getTexAndRectUtf8(renderer, _iconTex, 0, 0, icon.data(), "icons", ih,
                                              sdlTextColor);
                    iw = _iconTex.w();
                }
                else
                {
                    int w{};
                    int h{};
                    ih *= 0.9f;
                    iw = _iconTex.w() * ih / _iconTex.h();
                }
            }
            if (mCaption != "")
                iw += _pos.y * 0.15f;

            Vector2i iconPos = center.As<int>();
            iconPos.y -= 1;

            if (mIconPosition == IconPosition::LeftCentered)
            {
                iconPos.x -= _captionTex.w() * 0.5f;
                iconPos.x -= _iconTex.w() * 0.5f;
                textPos.x += _iconTex.w() * 0.5f;  // iw * 0.5f;
            }
            else if (mIconPosition == IconPosition::RightCentered)
            {
                textPos.x -= iw * 0.5f;
                iconPos.x += _captionTex.w() * 0.5f;
            }
            else if (mIconPosition == IconPosition::Left)
            {
                iconPos.x = getAbsoluteLeft() + 8;
            }
            else if (mIconPosition == IconPosition::Right)
            {
                iconPos.x = getAbsoluteLeft() + width() - iw - 8;
            }

            if (nvgIsFontIcon(mIcon))
                SDL_RenderCopy(renderer, _iconTex,
                               iconPos + getTextOffset() + Vector2i(0, -_iconTex.h() * 0.5f + 1));
            else
                SDL_RenderCopy(renderer, _iconTex, iconPos + getTextOffset() + Vector2i(0, -ih / 2));
        }

        SDL_RenderCopy(renderer, _captionTex, textPos + getTextOffset());
    }

    Vector2i Button::getTextOffset() const
    {
        int offset = mPushed ? 2 : 0;
        return Vector2i(offset, 1 + offset);
    }

    void Button::renderBodyTexture(NVGcontext*& ctx, int& realw, int& realh)
    {
        int ww = width();
        int hh = height();
        ctx = nvgCreateRT(NVG_DEBUG, ww + 2, hh + 2, 0);

        float pxRatio = 1.0f;
        realw = ww + 2;
        realh = hh + 2;
        nvgBeginFrame(ctx, realw, realh, pxRatio);

        NVGcolor gradTop = mTheme->mButtonGradientTopUnfocused.toNvgColor();
        NVGcolor gradBot = mTheme->mButtonGradientBotUnfocused.toNvgColor();

        if (mPushed)
        {
            gradTop = mTheme->mButtonGradientTopPushed.toNvgColor();
            gradBot = mTheme->mButtonGradientBotPushed.toNvgColor();
        }
        else if (mMouseFocus && mEnabled)
        {
            gradTop = mTheme->mButtonGradientTopFocused.toNvgColor();
            gradBot = mTheme->mButtonGradientBotFocused.toNvgColor();
        }

        nvgBeginPath(ctx);

        nvgRoundedRect(ctx, 1, 1.0f, ww - 2, hh - 2, mTheme->mButtonCornerRadius - 1);

        if (mBackgroundColor.a() != 0)
        {
            Color rgb = mBackgroundColor.rgb();
            rgb.setAlpha(1.f);
            nvgFillColor(ctx, rgb.toNvgColor());
            nvgFill(ctx);
            if (mPushed)
            {
                gradTop.a = gradBot.a = 0.8f;
            }
            else
            {
                float v = 1.0f - mBackgroundColor.a();
                gradTop.a = gradBot.a = mEnabled ? v : v * .5f + .5f;
            }
        }

        NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop, gradBot);

        nvgFillPaint(ctx, bg);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgStrokeWidth(ctx, 1.0f);
        nvgRoundedRect(ctx, 0.5f, (mPushed ? 0.5f : 1.5f), ww - 1, hh - 1 - (mPushed ? 0.0f : 1.0f),
                       mTheme->mButtonCornerRadius);
        nvgStrokeColor(ctx, mTheme->mBorderLight.toNvgColor());
        nvgStroke(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh - 2, mTheme->mButtonCornerRadius);
        nvgStrokeColor(ctx, mTheme->mBorderDark.toNvgColor());
        nvgStroke(ctx);

        nvgEndFrame(ctx);
    }

    void Button::drawTexture(AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer)
    {
        if (texture)
        {
            texture->perform(renderer);

            if (texture->tex.tex)
            {
                SDL_RenderCopy(renderer, texture->tex, absolutePosition());

                if (!current_texture_ || texture->id != current_texture_->id)
                    current_texture_ = texture;
            }
            else if (current_texture_)
            {
                SDL_RenderCopy(renderer, current_texture_->tex, absolutePosition());
            }
            else
                drawBodyTemp(renderer);
        }
        else
            drawBodyTemp(renderer);
    }

}