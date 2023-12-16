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
        m_standard_font_size = 16;
        m_button_font_size = 20;
        m_text_box_font_size = 20;
        m_window_corner_radius = 2;
        m_window_header_height = 30;
        m_window_drop_shadow_size = 10;
        m_button_corner_radius = 2;
        m_tab_border_width = 0.75f;
        m_tab_inner_margin = 5;
        m_tab_min_button_width = 30;
        m_tab_max_button_width = 180;
        m_tab_control_width = 20;
        m_tab_button_horizontal_padding = 10;
        m_tab_button_vertical_padding = 2;

        m_drop_shadow = Color(32, 32, 32, 255);
        m_transparent = Color(0, 0);
        m_border_dark = Color(29, 255);
        m_border_light = Color(92, 255);
        m_border_medium = Color(35, 255);
        m_text_color = Color(255, 160);
        m_disabled_text_color = Color(255, 80);
        m_text_color_shadow = Color(0, 160);
        m_icon_color = m_text_color;

        m_button_gradient_top_focused = Color(64, 255);
        m_button_gradient_bot_focused = Color(48, 255);
        m_button_gradient_top_unfocused = Color(74, 255);
        m_button_gradient_bot_unfocused = Color(58, 255);
        m_button_gradient_top_pushed = Color(41, 255);
        m_button_gradient_bot_pushed = Color(29, 255);

        /* Window-related */
        m_window_fill_unfocused = Color(43, 255);
        m_window_fill_focused = Color(45, 255);
        m_window_title_unfocused = Color(220, 160);
        m_window_title_focused = Color(255, 190);

        /* Slider */
        m_slider_knob_outer = Color(92, 255);
        m_slider_knob_inner = Color(220, 255);

        m_window_header_gradient_top = m_button_gradient_top_unfocused;
        m_window_header_gradient_bot = m_button_gradient_bot_unfocused;
        m_window_header_sep_top = m_border_light;
        m_window_header_sep_bot = m_border_dark;

        m_window_popup = Color(50, 255);
        m_window_popup_transparent = Color(50, 0);

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

    int Theme::get_text_bounds(const char* fontname, size_t ptsize, const char* text, int* w, int* h)
    {
        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return -1;

        SDL3::TTF_SizeText(font, text, w, h);
        return 0;
    }

    int Theme::get_utf8_bounds(const char* fontname, size_t ptsize, const char* text, int* w, int* h)
    {
        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return -1;

        SDL3::TTF_SizeUTF8(font, text, w, h);
        return 0;
    }

    int Theme::get_text_width(const char* fontname, size_t ptsize, const char* text)
    {
        int w, h;
        get_text_bounds(fontname, ptsize, text, &w, &h);
        return w;
    }

    int Theme::get_utf8_width(const char* fontname, size_t ptsize, const char* text)
    {
        SDL3::TTF_Font* font = getFont(fontname, ptsize);

        if (!font)
            return -1;

        int w, h;
        SDL3::TTF_SizeUTF8(font, text, &w, &h);
        return w;
    }

    void Theme::get_texture_and_rect(SDL3::SDL_Renderer* renderer, int x, int y, const char* text,
                                     const char* fontname, size_t ptsize,
                                     SDL3::SDL_Texture** texture, SDL3::SDL_Rect* rect,
                                     SDL3::SDL_Color* text_color)
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
            font, text, text_color ? *text_color : defColor);
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

    void Theme::get_texture_and_rect_utf8(SDL3::SDL_Renderer* renderer, int x, int y,
                                          const char* text, const char* fontname, size_t ptsize,
                                          SDL3::SDL_Texture** texture, SDL3::SDL_Rect* rect,
                                          SDL3::SDL_Color* text_color)
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
            font, text, text_color ? *text_color : defColor);
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

    std::string Theme::break_text(SDL3::SDL_Renderer* renderer, const char* string,
                                  const char* fontname, int ptsize, float breakRowWidth)
    {
        std::string _string(string);
        for (int i = 0; i < _string.size(); i++)
        {
            int slen = get_text_width(fontname, ptsize, _string.substr(0, i).c_str());
            if (slen >= breakRowWidth)
                return _string.substr(0, i);
        }

        return string;
    }

    void Theme::get_texture_and_rect_utf8(SDL3::SDL_Renderer* renderer, Texture& tx, int x, int y,
                                          const char* text, const char* fontname, size_t ptsize,
                                          const Color& text_color)
    {
        tx.dirty = false;
        SDL3::SDL_Color tColor = text_color.sdl_color();
        get_texture_and_rect_utf8(renderer, 0, 0, text, fontname, ptsize, &tx.tex, &tx.rrect,
                                  &tColor);
    }

    void SDL_RenderCopy(SDL3::SDL_Renderer* renderer, Texture& tx, const Vector2i& pos)
    {
        if (!tx.tex)
            return;

        SDL3::SDL_FRect rect{ pos.x, pos.y, tx.rrect.w, tx.rrect.h };
        SDL3::SDL_RenderTexture(renderer, tx.tex, nullptr, &rect);
    }
}
