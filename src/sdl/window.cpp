#include <memory>
#include <utility>

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
    window::window(SDL3::SDL_Window*&& other)
        : m_sdl_window{ other }
    {
        sdl_assert(other != nullptr, "constructed window from null SDL_Window");
        other = nullptr;
    }

    window::window(sdl::window&& other)
        : m_sdl_window{ other.m_sdl_window }
    {
        other.m_sdl_window = nullptr;
    }

    window::window(std::string title, ds::rect<i32> bounds, u32 flags)
        : m_sdl_window{ SDL3::SDL_CreateWindowWithPosition(
              title.data(), bounds.pt.x, bounds.pt.y, bounds.size.width, bounds.size.height, flags) }
    {
    }

    window::window(std::string title, ds::dimensions<i32> dims, u32 flags)
        : m_sdl_window{ SDL3::SDL_CreateWindow(title.data(), dims.width, dims.height, flags) }
    {
    }

    window::~window()
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }
    }

    const window& window::operator=(sdl::window&& other)
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }

        std::swap(m_sdl_window, other.m_sdl_window);
        return *this;
    }

    const window& window::maximize()
    {
        SDL3::SDL_MaximizeWindow(m_sdl_window);
        return *this;
    }

    const window& window::minimize()
    {
        SDL3::SDL_MinimizeWindow(m_sdl_window);
        return *this;
    }

    const window& window::hide()
    {
        SDL3::SDL_HideWindow(m_sdl_window);
        return *this;
    }

    const window& window::restore()
    {
        SDL3::SDL_RestoreWindow(m_sdl_window);
        return *this;
    }

    const window& window::raise()
    {
        SDL3::SDL_RaiseWindow(m_sdl_window);
        return *this;
    }

    const window& window::show()
    {
        SDL3::SDL_ShowWindow(m_sdl_window);
        return *this;
    }

    ///////////////////////////////////////////////////
    const window& window::set_grab(bool grabbed)
    {
        SDL3::SDL_SetWindowGrab(m_sdl_window, sdl::boolean(grabbed));
        return *this;
    }

    const window& window::set_bordered(bool bordered)
    {
        SDL3::SDL_SetWindowBordered(m_sdl_window, sdl::boolean(bordered));
        return *this;
    }

    const window& window::set_resizable(bool resizable)
    {
        SDL3::SDL_SetWindowResizable(m_sdl_window, sdl::boolean(resizable));
        return *this;
    }

    const window& window::set_fullscreen(bool fullscreen)
    {
        i32 result{ SDL3::SDL_SetWindowFullscreen(m_sdl_window, sdl::boolean(fullscreen)) };
        runtime_assert(result == 0, "Failed to set window to fullscreen");
        return *this;
    }

    const window& window::set_opacity(float opacity)
    {
        i32 result{ SDL3::SDL_SetWindowOpacity(m_sdl_window, opacity) };
        runtime_assert(result != 0, "failed to set window opacity");
        return *this;
    }

    const window& window::set_title(std::string title)
    {
        SDL3::SDL_SetWindowTitle(m_sdl_window, title.c_str());
        return *this;
    }

    const window& window::set_position(ds::point<i32> pos)
    {
        SDL3::SDL_SetWindowPosition(m_sdl_window, pos.x, pos.y);
        return *this;
    }

    const window& window::set_size(ds::dimensions<i32> size)
    {
        i32 result{ SDL3::SDL_SetWindowSize(m_sdl_window, size.width, size.height) };
        runtime_assert(result == 0, "failed to set window size");
        return *this;
    }

    const window& window::set_min_size(ds::dimensions<i32> size)
    {
        SDL3::SDL_SetWindowMinimumSize(m_sdl_window, size.width, size.height);
        return *this;
    }

    const window& window::set_max_size(ds::dimensions<i32> size)
    {
        SDL3::SDL_SetWindowMaximumSize(m_sdl_window, size.width, size.height);
        return *this;
    }

    SDL3::SDL_WindowFlags window::get_flags() const
    {
        const u32 flags = SDL3::SDL_GetWindowFlags(m_sdl_window);
        return static_cast<SDL3::SDL_WindowFlags>(flags);
    }

    bool window::is_valid() const
    {
        return this->sdl_handle() != nullptr;
    }

    SDL3::SDL_Window* window::sdl_handle() const
    {
        return m_sdl_window;
    }

    ds::dimensions<i32> window::get_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        SDL3::SDL_GetWindowSize(m_sdl_window, &size.width, &size.height);
        return size;
    }

    ds::dimensions<i32> window::get_render_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        SDL3::SDL_GetWindowSizeInPixels(m_sdl_window, &size.width, &size.height);
        return size;
    }

    std::string window::get_title() const
    {
        return std::string{ SDL3::SDL_GetWindowTitle(m_sdl_window) };
    }

    ds::point<i32> window::get_position() const
    {
        ds::point<i32> pos{ 0, 0 };
        SDL3::SDL_GetWindowPosition(m_sdl_window, &pos.x, &pos.y);
        return pos;
    }

    ds::dimensions<i32> window::get_min_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        SDL3::SDL_GetWindowMinimumSize(m_sdl_window, &size.width, &size.height);
        return size;
    }

    ds::dimensions<i32> window::get_max_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        SDL3::SDL_GetWindowMaximumSize(m_sdl_window, &size.width, &size.height);
        return size;
    }

    bool window::get_grab() const
    {
        auto grab{ SDL3::SDL_GetWindowGrab(m_sdl_window) == SDL3::SDL_TRUE };
        return sdl::boolean(grab);
    }

    SDL3::SDL_DisplayID window::get_display() const
    {
        SDL3::SDL_DisplayID id{ SDL3::SDL_GetDisplayForWindow(m_sdl_window) };
        runtime_assert(id < 0, "failed to set window display idx");
        return id;
    }

    SDL3::SDL_DisplayMode window::get_display_mode() const
    {
        const SDL3::SDL_DisplayMode* mode{ SDL3::SDL_GetWindowFullscreenMode(m_sdl_window) };
        runtime_assert(mode == nullptr, "failed to get window display mode");

        SDL3::SDL_DisplayMode ret{};
        SDL_memcpy(&ret, mode, sizeof(ret));
        return ret;
    }

    f32 window::get_opacity() const
    {
        f32 opacity{ 0.0f };
        i32 result{ SDL3::SDL_GetWindowOpacity(m_sdl_window, &opacity) };
        runtime_assert(result == -1, "failed to get window opacity");
        return opacity;
    }

}
