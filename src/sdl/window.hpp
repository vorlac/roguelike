#pragma once

#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"
#include "sdl/renderer.hpp"
#include "sdl/utils.hpp"

namespace SDL3
{
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
}

namespace rl::sdl
{
    class window
    {
    public:
        window(SDL3::SDL_Window*&& other);
        window(sdl::window&& other);
        window(std::string title = "SDL3", ds::rect<i32> bounds = window::DEFAULT_SCREEN_RECT,
               u32 flags = SDL3::SDL_WINDOW_RESIZABLE);
        window(std::string title, ds::dimensions<i32> dims = window::DEFAULT_SCREEN_DIMS,
               u32 flags = SDL3::SDL_WINDOW_RESIZABLE);
        ~window();

        const window& operator=(sdl::window&& other);
        SDL3::SDL_Window* sdl_handle() const;
        bool is_valid() const;

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

    protected:
        inline static constexpr ds::point<i32> DEFAULT_SCREEN_POS = {
            SDL_WINDOWPOS_CENTERED_MASK,
            SDL_WINDOWPOS_CENTERED_MASK,
        };

        inline static constexpr ds::dimensions<i32> DEFAULT_SCREEN_DIMS = { 640, 480 };

        inline static constexpr ds::rect<i32> DEFAULT_SCREEN_RECT = {
            window::DEFAULT_SCREEN_POS,
            window::DEFAULT_SCREEN_DIMS,
        };

    private:
        SDL3::SDL_Window* m_sdl_window{ nullptr };
    };
}
