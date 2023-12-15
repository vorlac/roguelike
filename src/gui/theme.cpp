#include <map>
#include <string>

#include "gui/resources.hpp"
#include "gui/theme.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3_ttf/SDL_ttf.h>
SDL_C_LIB_END

#pragma warning(disable : 4838)

namespace rl::gui {

    namespace internal {
        std::map<std::string, SDL3::TTF_Font*> fonts{};
    }

    Theme::Theme(SDL3::SDL_Renderer* ctx)
    {
        mStandardFontSize = 16;
        mButtonFontSize = 20;
        mTextBoxFontSize = 20;
        mWindowCornerRadius = 2;
        mWindowHeaderHeight = 30;
        mWindowDropShadowSize = 10;
        mButtonCornerRadius = 2;
        mTabBorderWidth = 0.75f;
        mTabInnerMargin = 5;
        mTabMinButtonWidth = 30;
        mTabMaxButtonWidth = 180;
        mTabControlWidth = 20;
        mTabButtonHorizontalPadding = 10;
        mTabButtonVerticalPadding = 2;

        mDropShadow = Color(32, 32, 32, 255);
        mTransparent = Color(0, 0);
        mBorderDark = Color(29, 255);
        mBorderLight = Color(92, 255);
        mBorderMedium = Color(35, 255);
        mTextColor = Color(255, 160);
        mDisabledTextColor = Color(255, 80);
        mTextColorShadow = Color(0, 160);
        mIconColor = mTextColor;

        mButtonGradientTopFocused = Color(64, 255);
        mButtonGradientBotFocused = Color(48, 255);
        mButtonGradientTopUnfocused = Color(74, 255);
        mButtonGradientBotUnfocused = Color(58, 255);
        mButtonGradientTopPushed = Color(41, 255);
        mButtonGradientBotPushed = Color(29, 255);

        /* Window-related */
        mWindowFillUnfocused = Color(43, 255);
        mWindowFillFocused = Color(45, 255);
        mWindowTitleUnfocused = Color(220, 160);
        mWindowTitleFocused = Color(255, 190);

        /* Slider */
        mSliderKnobOuter = Color(92, 255);
        mSliderKnobInner = Color(220, 255);

        mWindowHeaderGradientTop = mButtonGradientTopUnfocused;
        mWindowHeaderGradientBot = mButtonGradientBotUnfocused;
        mWindowHeaderSepTop = mBorderLight;
        mWindowHeaderSepBot = mBorderDark;

        mWindowPopup = Color(50, 255);
        mWindowPopupTransparent = Color(50, 0);

        SDL3::TTF_Init();
    }

    SDL3::TTF_Font* getFont(const char* fontname, size_t ptsize)
    {
        std::string fullFontName = fontname;
        fullFontName += "_";
        fullFontName += std::to_string(ptsize);

        SDL3::TTF_Font* font = nullptr;
        auto fontIt = internal::fonts.find(fullFontName);
        if (fontIt == internal::fonts.end())
        {
            SDL3::SDL_RWops* rw = nullptr;
            std::string tmpFontname = fontname;
            if (tmpFontname == "sans")
                rw = SDL3::SDL_RWFromMem(roboto_regular_ttf, roboto_regular_ttf_size);
            else if (tmpFontname == "sans-bold")
                rw = SDL3::SDL_RWFromMem(roboto_bold_ttf, roboto_bold_ttf_size);
            else if (tmpFontname == "icons")
                rw = SDL3::SDL_RWFromMem(entypo_ttf, entypo_ttf_size);

            SDL3::TTF_Font* newFont = SDL3::TTF_OpenFontRW(rw, false, static_cast<int>(ptsize));
            internal::fonts[fullFontName] = newFont;
            font = newFont;
        }
        else
        {
            font = fontIt->second;
        }

        return font;
    }

    int Theme::getTextBounds(const char* fontname, size_t ptsize, const char* text, int* w, int* h)
    {
        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return -1;

        SDL3::TTF_SizeText(font, text, w, h);
        return 0;
    }

    int Theme::getUtf8Bounds(const char* fontname, size_t ptsize, const char* text, int* w, int* h)
    {
        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return -1;

        SDL3::TTF_SizeUTF8(font, text, w, h);
        return 0;
    }

    int Theme::getTextWidth(const char* fontname, size_t ptsize, const char* text)
    {
        int w, h;
        getTextBounds(fontname, ptsize, text, &w, &h);
        return w;
    }

    int Theme::getUtf8Width(const char* fontname, size_t ptsize, const char* text)
    {
        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return -1;

        int w, h;
        SDL3::TTF_SizeUTF8(font, text, &w, &h);
        return w;
    }

    void Theme::getTexAndRect(SDL3::SDL_Renderer* renderer, int x, int y, const char* text,
                              const char* fontname, size_t ptsize, SDL3::SDL_Texture** texture,
                              SDL3::SDL_Rect* rect, SDL3::SDL_Color* textColor)
    {
        int text_width;
        int text_height;

        if (*texture != nullptr)
            SDL_DestroyTexture(*texture);

        SDL3::SDL_Color defColor{ 255, 255, 255, 0 };

        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return;

        SDL3::SDL_Surface* surface = SDL3::TTF_RenderText_Blended(
            font, text, textColor ? *textColor : defColor);
        if (!surface)
        {
            rect->x = x;
            rect->y = y;
            rect->w = 0;
            rect->h = 0;
            *texture = nullptr;
            return;
        }

        *texture = SDL_CreateTextureFromSurface(renderer, surface);
        text_width = surface->w;
        text_height = surface->h;
        SDL3::SDL_DestroySurface(surface);
        rect->x = x;
        rect->y = y;
        rect->w = text_width;
        rect->h = text_height;
    }

    void Theme::getTexAndRectUtf8(SDL3::SDL_Renderer* renderer, int x, int y, const char* text,
                                  const char* fontname, size_t ptsize, SDL3::SDL_Texture** texture,
                                  SDL3::SDL_Rect* rect, SDL3::SDL_Color* textColor)
    {
        int text_width;
        int text_height;

        if (*texture != nullptr)
            SDL_DestroyTexture(*texture);

        SDL3::SDL_Color defColor{ 255, 255, 255, 0 };

        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return;

        SDL3::SDL_Surface* surface = SDL3::TTF_RenderUTF8_Blended(
            font, text, textColor ? *textColor : defColor);
        if (!surface)
        {
            rect->x = x;
            rect->y = y;
            rect->w = 0;
            rect->h = 0;
            *texture = nullptr;
            return;
        }

        *texture = SDL_CreateTextureFromSurface(renderer, surface);
        text_width = surface->w;
        text_height = surface->h;
        SDL3::SDL_DestroySurface(surface);
        rect->x = x;
        rect->y = y;
        rect->w = text_width;
        rect->h = text_height;
    }

    std::string Theme::breakText(SDL3::SDL_Renderer* renderer, const char* string,
                                 const char* fontname, int ptsize, float breakRowWidth)
    {
        std::string _string(string);
        for (int i = 0; i < _string.size(); i++)
        {
            int slen = getTextWidth(fontname, ptsize, _string.substr(0, i).c_str());
            if (slen >= breakRowWidth)
                return _string.substr(0, i);
        }

        return string;
    }

    void Theme::getTexAndRectUtf8(SDL3::SDL_Renderer* renderer, Texture& tx, int x, int y,
                                  const char* text, const char* fontname, size_t ptsize,
                                  const Color& textColor)
    {
        tx.dirty = false;
        SDL3::SDL_Color tColor = textColor.toSdlColor();
        getTexAndRectUtf8(renderer, 0, 0, text, fontname, ptsize, &tx.tex, &tx.rrect, &tColor);
    }

    void SDL_RenderCopy(SDL3::SDL_Renderer* renderer, Texture& tx, const Vector2i& pos)
    {
        if (!tx.tex)
            return;

        SDL3::SDL_FRect rect{ pos.x, pos.y, tx.rrect.w, tx.rrect.h };
        SDL3::SDL_RenderTexture(renderer, tx.tex, nullptr, &rect);
    }

}
