#pragma once

#include <mutex>
#include <string>

#include "gui/common.hpp"

SDL_C_LIB_BEGIN
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Rect;
SDL_C_LIB_END

namespace rl::gui {
    struct Texture
    {
        SDL3::SDL_Texture* tex{ nullptr };
        SDL3::SDL_Rect rrect{};
        bool dirty{ false };

        inline int w() const
        {
            return rrect.w;
        }

        inline int h() const
        {
            return rrect.h;
        }

        constexpr operator SDL3::SDL_Texture*()
        {
            return this->tex;
        }
    };

    void SDL_RenderCopy(SDL3::SDL_Renderer* renderer, Texture& tex, const Vector2i& pos);

    /*
     * @brief Storage class for basic theme-related properties.
     */
    class Theme : public Object
    {
    public:
        Theme(SDL3::SDL_Renderer* ctx);

        // Spacing-related parameters
        int m_standard_font_size;
        int m_button_font_size;
        int m_text_box_font_size;
        int m_window_corner_radius;
        int m_window_header_height;
        int m_window_drop_shadow_size;
        int m_button_corner_radius;
        float m_tab_border_width;
        int m_tab_inner_margin;
        int m_tab_min_button_width;
        int m_tab_max_button_width;
        int m_tab_control_width;
        int m_tab_button_horizontal_padding;
        int m_tab_button_vertical_padding;

        std::mutex m_load_mutex{};

        // Generic colors
        Color m_drop_shadow;
        Color m_transparent;
        Color m_border_dark;
        Color m_border_light;
        Color m_border_medium;
        Color m_text_color;
        Color m_disabled_text_color;
        Color m_text_color_shadow;
        Color m_icon_color;

        // Button colors
        Color m_button_gradient_top_focused;
        Color m_button_gradient_bot_focused;
        Color m_button_gradient_top_unfocused;
        Color m_button_gradient_bot_unfocused;
        Color m_button_gradient_top_pushed;
        Color m_button_gradient_bot_pushed;

        // Window colors
        Color m_window_fill_unfocused;
        Color m_window_fill_focused;
        Color m_window_title_unfocused;
        Color m_window_title_focused;

        // Slider coloes
        Color m_slider_knob_outer;
        Color m_slider_knob_inner;

        Color m_window_header_gradient_top;
        Color m_window_header_gradient_bot;
        Color m_window_header_sep_top;
        Color m_window_header_sep_bot;

        Color m_window_popup;
        Color m_window_popup_transparent;

        void get_texture_and_rect(SDL3::SDL_Renderer* renderer, int x, int y, const char* text,
                                  const char* fontname, size_t ptsize, SDL3::SDL_Texture** texture,
                                  SDL3::SDL_Rect* rect, SDL3::SDL_Color* text_color);

        void get_texture_and_rect_utf8(SDL3::SDL_Renderer* renderer, int x, int y, const char* text,
                                       const char* fontname, size_t ptsize,
                                       SDL3::SDL_Texture** texture, SDL3::SDL_Rect* rect,
                                       SDL3::SDL_Color* text_color);

        std::string break_text(SDL3::SDL_Renderer* renderer, const char* string,
                               const char* fontname, int ptsize, float breakRowWidth);

        int get_text_width(const char* fontname, size_t ptsize, const char* text);
        int get_utf8_width(const char* fontname, size_t ptsize, const char* text);
        int get_text_bounds(const char* fontname, size_t ptsize, const char* text, int* w, int* h);
        int get_utf8_bounds(const char* fontname, size_t ptsize, const char* text, int* w, int* h);

        void get_texture_and_rect_utf8(SDL3::SDL_Renderer* renderer, Texture& tx, int x, int y,
                                       const char* text, const char* fontname, size_t ptsize,
                                       const Color& text_color);

    protected:
        virtual ~Theme() override
        {
        }
    };
}
