#pragma once

#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"
#include "sdl/renderer.hpp"
#include "sdl/utils.hpp"

namespace SDL3 {
#include <SDL3/SDL_video.h>
}

namespace rl::sdl {
    class window
    {
        window() = delete;
        window(const window& window) = delete;
        window(window& window) = delete;

    public:
        explicit window(SDL3::SDL_Window*&& other) noexcept;
        explicit window(sdl::window&& other) noexcept;
        explicit window(const std::string& title = window::defaults::Title,
                        const ds::rect<i32>& bounds = window::defaults::Rect,
                        SDL3::SDL_WindowFlags flags = window::defaults::Properties);
        explicit window(const std::string& title, const ds::dimensions<i32>& dims,
                        SDL3::SDL_WindowFlags flags = window::defaults::Properties);
        ~window();

    public:
        const window& maximize();
        const window& minimize();
        const window& hide();
        const window& restore();
        const window& raise();
        const window& show();

    public:
        const window& set_grab(bool grabbed);
        const window& set_bordered(bool bordered);
        const window& set_resizable(bool resizable);
        const window& set_fullscreen(bool fullscreen);
        const window& set_opacity(float opacity);
        const window& set_title(std::string title);
        const window& set_position(ds::point<i32> pos);
        const window& set_size(ds::dimensions<i32> size);
        const window& set_min_size(ds::dimensions<i32> size);
        const window& set_max_size(ds::dimensions<i32> size);

    public:
        bool get_grab() const;
        f32 get_opacity() const;
        std::string get_title() const;
        SDL3::SDL_DisplayID get_display() const;
        SDL3::SDL_WindowFlags get_flags() const;
        SDL3::SDL_DisplayMode get_display_mode() const;
        ds::dimensions<i32> get_size() const;
        ds::dimensions<i32> get_render_size() const;
        ds::dimensions<i32> get_min_size() const;
        ds::dimensions<i32> get_max_size() const;
        ds::point<i32> get_position() const;

        const window& operator=(sdl::window&& other) noexcept;
        SDL3::SDL_Window* sdl_handle() const;
        bool is_valid() const;

    public:
        using flag_t = SDL3::SDL_WindowFlags;

        struct flag
        {
            constexpr static inline flag_t Fullscreen = SDL3::SDL_WINDOW_FULLSCREEN;
            constexpr static inline flag_t OpenGL = SDL3::SDL_WINDOW_OPENGL;
            constexpr static inline flag_t Occluded = SDL3::SDL_WINDOW_OCCLUDED;
            constexpr static inline flag_t Hidden = SDL3::SDL_WINDOW_HIDDEN;
            constexpr static inline flag_t Borderless = SDL3::SDL_WINDOW_BORDERLESS;
            constexpr static inline flag_t Resizable = SDL3::SDL_WINDOW_RESIZABLE;
            constexpr static inline flag_t Minimized = SDL3::SDL_WINDOW_MINIMIZED;
            constexpr static inline flag_t Maximized = SDL3::SDL_WINDOW_MAXIMIZED;
            constexpr static inline flag_t MouseGrabbed = SDL3::SDL_WINDOW_MOUSE_GRABBED;
            constexpr static inline flag_t InputFocus = SDL3::SDL_WINDOW_INPUT_FOCUS;
            constexpr static inline flag_t MouseFocus = SDL3::SDL_WINDOW_MOUSE_FOCUS;
            constexpr static inline flag_t Foreign = SDL3::SDL_WINDOW_FOREIGN;
            constexpr static inline flag_t HighDPI = SDL3::SDL_WINDOW_HIGH_PIXEL_DENSITY;
            constexpr static inline flag_t MouseCapture = SDL3::SDL_WINDOW_MOUSE_CAPTURE;
            constexpr static inline flag_t AlwaysOnTop = SDL3::SDL_WINDOW_ALWAYS_ON_TOP;
            constexpr static inline flag_t Utility = SDL3::SDL_WINDOW_UTILITY;
            constexpr static inline flag_t Tooltip = SDL3::SDL_WINDOW_TOOLTIP;
            constexpr static inline flag_t PopupMenu = SDL3::SDL_WINDOW_POPUP_MENU;
            constexpr static inline flag_t KeyboardGrabbed = SDL3::SDL_WINDOW_KEYBOARD_GRABBED;
            constexpr static inline flag_t Vulkan = SDL3::SDL_WINDOW_VULKAN;
            constexpr static inline flag_t Metal = SDL3::SDL_WINDOW_METAL;
            constexpr static inline flag_t Transparent = SDL3::SDL_WINDOW_TRANSPARENT;
            constexpr static inline flag_t NotFocusable = SDL3::SDL_WINDOW_NOT_FOCUSABLE;
        };

        struct defaults
        {
            constexpr static inline auto Properties = window::flag_t(
                flag::HighDPI | flag::MouseFocus | flag::Vulkan | flag::Resizable);

            constexpr static inline ds::point<i32> Position = {
                SDL_WINDOWPOS_CENTERED_MASK,
                SDL_WINDOWPOS_CENTERED_MASK,
            };

            constexpr static inline ds::dimensions<i32> Size = {
                1024,
                768,
            };

            constexpr static inline ds::rect<i32> Rect = {
                window::defaults::Position,
                window::defaults::Size,
            };

            constexpr static inline auto Title{ "SDL3 Roguelite" };
        };

    private:
        SDL3::SDL_Window* m_sdl_window{ nullptr };
    };
}
