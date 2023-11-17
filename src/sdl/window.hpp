#pragma once

#include <memory>
#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"
#include "sdl/utils.hpp"

namespace SDL3 {
#include <SDL3/SDL_video.h>
}

namespace rl::sdl {
    class renderer;

    class window
    {
        window() = delete;
        window(const window& window) = delete;
        window(window& window) = delete;

    public:
        explicit window(SDL3::SDL_Window*&& other) noexcept;
        explicit window(sdl::window&& other) noexcept;
        window(const std::string& title, const ds::dimensions<i32>& dims = window::defaults::Size,
               SDL3::SDL_WindowFlags flags = window::defaults::Properties);
        ~window();

    public:
        bool maximize();
        bool minimize();
        bool hide();
        bool restore();
        bool raise();
        bool show();

        bool set_grab(bool grabbed);
        bool set_bordered(bool bordered);
        bool set_resizable(bool resizable);
        bool set_fullscreen(bool fullscreen);
        bool set_opacity(float opacity);
        bool set_title(std::string title);
        bool set_position(ds::point<i32> pos);
        bool set_size(ds::dimensions<i32> size);
        bool set_min_size(ds::dimensions<i32> size);
        bool set_max_size(ds::dimensions<i32> size);

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

        std::shared_ptr<sdl::renderer> renderer() const;
        const window& operator=(sdl::window&& other) noexcept;
        SDL3::SDL_Window* sdl_handle() const;
        bool is_valid() const;

    public:
        struct flag
        {
            using type = SDL3::SDL_WindowFlags;

            constexpr static inline type Fullscreen = SDL3::SDL_WINDOW_FULLSCREEN;
            constexpr static inline type OpenGL = SDL3::SDL_WINDOW_OPENGL;
            constexpr static inline type Occluded = SDL3::SDL_WINDOW_OCCLUDED;
            constexpr static inline type Hidden = SDL3::SDL_WINDOW_HIDDEN;
            constexpr static inline type Borderless = SDL3::SDL_WINDOW_BORDERLESS;
            constexpr static inline type Resizable = SDL3::SDL_WINDOW_RESIZABLE;
            constexpr static inline type Minimized = SDL3::SDL_WINDOW_MINIMIZED;
            constexpr static inline type Maximized = SDL3::SDL_WINDOW_MAXIMIZED;
            constexpr static inline type MouseGrabbed = SDL3::SDL_WINDOW_MOUSE_GRABBED;
            constexpr static inline type InputFocus = SDL3::SDL_WINDOW_INPUT_FOCUS;
            constexpr static inline type MouseFocus = SDL3::SDL_WINDOW_MOUSE_FOCUS;
            constexpr static inline type External = SDL3::SDL_WINDOW_EXTERNAL;
            constexpr static inline type HighDPI = SDL3::SDL_WINDOW_HIGH_PIXEL_DENSITY;
            constexpr static inline type MouseCapture = SDL3::SDL_WINDOW_MOUSE_CAPTURE;
            constexpr static inline type AlwaysOnTop = SDL3::SDL_WINDOW_ALWAYS_ON_TOP;
            constexpr static inline type Utility = SDL3::SDL_WINDOW_UTILITY;
            constexpr static inline type Tooltip = SDL3::SDL_WINDOW_TOOLTIP;
            constexpr static inline type PopupMenu = SDL3::SDL_WINDOW_POPUP_MENU;
            constexpr static inline type KeyboardGrabbed = SDL3::SDL_WINDOW_KEYBOARD_GRABBED;
            constexpr static inline type Vulkan = SDL3::SDL_WINDOW_VULKAN;
            constexpr static inline type Metal = SDL3::SDL_WINDOW_METAL;
            constexpr static inline type Transparent = SDL3::SDL_WINDOW_TRANSPARENT;
            constexpr static inline type NotFocusable = SDL3::SDL_WINDOW_NOT_FOCUSABLE;
        };

        struct defaults
        {
            constexpr static inline window::flag::type Properties = {
                // flag::HighDPI |     //
                // flag::MouseFocus |  //
                // flag::InputFocus |  //
                // flag::Vulkan | //
                window::flag::Resizable
            };

            constexpr static inline ds::point<i32> Position = {
                SDL_WINDOWPOS_CENTERED_MASK,
                SDL_WINDOWPOS_CENTERED_MASK,
            };

            constexpr static inline ds::dimensions<i32> Size = {
                1920,
                1080,
            };

            constexpr static inline ds::rect<i32> Rect = {
                window::defaults::Position,
                window::defaults::Size,
            };

            constexpr static inline auto Title{ "SDL3 Roguelite" };
        };

    private:
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        std::shared_ptr<sdl::renderer> m_renderer{ nullptr };
    };
}
