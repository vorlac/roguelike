#pragma once

#include <bitset>
#include <memory>
#include <string>

#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "sdl/utils.hpp"
#include "utils/assert.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    class Renderer;

    class Window
    {
    public:
        struct Properties : public std::bitset<sizeof(u32) * 8>
        {
            using sdl_type = SDL3::SDL_WindowFlags;

            enum Flag : std::underlying_type_t<sdl_type> {
                None = 0,
                Fullscreen = SDL3::SDL_WINDOW_FULLSCREEN,
                OpenGL = SDL3::SDL_WINDOW_OPENGL,
                Occluded = SDL3::SDL_WINDOW_OCCLUDED,
                Hidden = SDL3::SDL_WINDOW_HIDDEN,
                Borderless = SDL3::SDL_WINDOW_BORDERLESS,
                Resizable = SDL3::SDL_WINDOW_RESIZABLE,
                Minimized = SDL3::SDL_WINDOW_MINIMIZED,
                Maximized = SDL3::SDL_WINDOW_MAXIMIZED,
                MouseGrabbed = SDL3::SDL_WINDOW_MOUSE_GRABBED,
                InputFocus = SDL3::SDL_WINDOW_INPUT_FOCUS,
                MouseFocus = SDL3::SDL_WINDOW_MOUSE_FOCUS,
                External = SDL3::SDL_WINDOW_EXTERNAL,
                HighDPI = SDL3::SDL_WINDOW_HIGH_PIXEL_DENSITY,
                MouseCapture = SDL3::SDL_WINDOW_MOUSE_CAPTURE,
                AlwaysOnTop = SDL3::SDL_WINDOW_ALWAYS_ON_TOP,
                Utility = SDL3::SDL_WINDOW_UTILITY,
                Tooltip = SDL3::SDL_WINDOW_TOOLTIP,
                PopupMenu = SDL3::SDL_WINDOW_POPUP_MENU,
                KeyboardGrabbed = SDL3::SDL_WINDOW_KEYBOARD_GRABBED,
                Vulkan = SDL3::SDL_WINDOW_VULKAN,
                Metal = SDL3::SDL_WINDOW_METAL,
                Transparent = SDL3::SDL_WINDOW_TRANSPARENT,
                NotFocusable = SDL3::SDL_WINDOW_NOT_FOCUSABLE,
            };

            constexpr inline operator sdl_type()
            {
                return static_cast<sdl_type>(this->to_ulong());
            }

            constexpr inline operator sdl_type() const
            {
                return static_cast<sdl_type>(this->to_ulong());
            }
        };

        constexpr static inline Window::Properties DEFAULT_PROPERTY_FLAGS = {
            Properties::Flag::HighDPI |     //
            Properties::Flag::InputFocus |  //
            Properties::Flag::MouseFocus |  //
            Properties::Flag::Resizable |   //
            Properties::Flag::Occluded |    //
            Properties::Flag::OpenGL        //
            // Properties::Flag::Vulkan
        };

        constexpr static inline ds::point<i32> DEFAULT_POSITION = {
            SDL_WINDOWPOS_CENTERED_MASK,
            SDL_WINDOWPOS_CENTERED_MASK,
        };

        constexpr static inline ds::dimensions<i32> DEFAULT_SIZE = {
            1920,
            1080,
        };

    public:
        explicit Window() = delete;
        explicit Window(Window&& window) = delete;
        explicit Window(const Window& window) = delete;
        explicit Window(SDL3::SDL_Window* other) = delete;

        explicit Window(std::string title, const ds::dimensions<i32>& dims = DEFAULT_SIZE,
                        Properties flags = DEFAULT_PROPERTY_FLAGS);

        ~Window();

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

        std::shared_ptr<sdl::Renderer> renderer() const;
        const Window& operator=(sdl::Window&& other) noexcept;
        SDL3::SDL_Window* sdl_handle() const;
        bool is_valid() const;

    private:
        Properties m_properties{ Properties::Flag::None };
        SDL3::SDL_Window* m_sdl_window{ nullptr };
        std::shared_ptr<sdl::Renderer> m_renderer{ nullptr };
    };
}
