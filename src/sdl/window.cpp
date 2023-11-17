#include <memory>
#include <utility>

#include <fmt/format.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer.hpp"
#include "sdl/utils.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    window::window(SDL3::SDL_Window*&& other) noexcept
        : m_sdl_window{ other }
        , m_renderer{ new sdl::renderer(*this, renderer::defaults::Driver) }
    {
        sdl_assert(other != nullptr, "constructed window from null SDL_Window");
        other = nullptr;
    }

    window::window(sdl::window&& other) noexcept
        : m_sdl_window{ other.m_sdl_window }
        , m_renderer{ new sdl::renderer(*this, renderer::defaults::Driver) }
    {
        sdl_assert(m_sdl_window != nullptr, "constructed window from null SDL_Window");
        other.m_sdl_window = nullptr;
    }

    window::window(const std::string& title, const ds::dimensions<i32>& dims,
                   SDL3::SDL_WindowFlags flags)
        : m_sdl_window{ SDL3::SDL_CreateWindow(title.c_str(), dims.width, dims.height, flags) }
        , m_renderer{ new sdl::renderer(*this, renderer::defaults::Driver) }
    {
        sdl_assert(m_sdl_window != nullptr, "constructed window from null SDL_Window");
    }

    window::~window()
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }
    }

    const window& window::operator=(sdl::window&& other) noexcept
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }

        std::swap(m_sdl_window, other.m_sdl_window);
        return *this;
    }

    bool window::maximize()
    {
        i32 result = SDL3::SDL_MaximizeWindow(m_sdl_window);
        sdl_assert(result == 0, "failed to maximize");
        return result == 0;
    }

    bool window::minimize()
    {
        i32 result = SDL3::SDL_MinimizeWindow(m_sdl_window);
        sdl_assert(result == 0, "failed to minimize");
        return result == 0;
    }

    bool window::hide()
    {
        i32 result = SDL3::SDL_HideWindow(m_sdl_window);
        sdl_assert(result == 0, "failed hiding");
        return result == 0;
    }

    bool window::restore()
    {
        i32 result = SDL3::SDL_RestoreWindow(m_sdl_window);
        sdl_assert(result == 0, "failed restoring");
        return result == 0;
    }

    bool window::raise()
    {
        i32 result = SDL3::SDL_RaiseWindow(m_sdl_window);
        sdl_assert(result == 0, "failed raising");
        return result == 0;
    }

    bool window::show()
    {
        i32 result = SDL3::SDL_ShowWindow(m_sdl_window);
        sdl_assert(result == 0, "failed to show");
        return result == 0;
    }

    bool window::set_grab(bool grabbed)
    {
        i32 result = SDL3::SDL_SetWindowGrab(m_sdl_window, sdl::boolean(grabbed));
        sdl_assert(result == 0, "failed to set grab");
        return result == 0;
    }

    bool window::set_bordered(bool bordered)
    {
        i32 result = SDL3::SDL_SetWindowBordered(m_sdl_window, sdl::boolean(bordered));
        sdl_assert(result == 0, "failed to set bordered");
        return result == 0;
    }

    bool window::set_resizable(bool resizable)
    {
        i32 result = SDL3::SDL_SetWindowResizable(m_sdl_window, sdl::boolean(resizable));
        sdl_assert(result == 0, "failed to set resizeable");
        return result == 0;
    }

    bool window::set_fullscreen(bool fullscreen)
    {
        i32 result{ SDL3::SDL_SetWindowFullscreen(m_sdl_window, sdl::boolean(fullscreen)) };
        runtime_assert(result == 0, "Failed to set window to fullscreen");
        return result == 0;
    }

    bool window::set_opacity(float opacity)
    {
        i32 result = SDL3::SDL_SetWindowOpacity(m_sdl_window, opacity);
        runtime_assert(result != 0, "failed to set window opacity");
        return result == 0;
    }

    bool window::set_title(std::string title)
    {
        i32 result = SDL3::SDL_SetWindowTitle(m_sdl_window, title.c_str());
        sdl_assert(result == 0, "failed to set title");
        return result == 0;
    }

    bool window::set_position(ds::point<i32> pos)
    {
        i32 result = SDL3::SDL_SetWindowPosition(m_sdl_window, pos.x, pos.y);
        sdl_assert(result == 0, "failed to set position");
        return result == 0;
    }

    bool window::set_size(ds::dimensions<i32> size)
    {
        i32 result{ SDL3::SDL_SetWindowSize(m_sdl_window, size.width, size.height) };
        runtime_assert(result == 0, "failed to set size");
        return result == 0;
    }

    bool window::set_min_size(ds::dimensions<i32> size)
    {
        i32 result = SDL3::SDL_SetWindowMinimumSize(m_sdl_window, size.width, size.height);
        sdl_assert(result == 0, "failed to set min size");
        return result == 0;
    }

    bool window::set_max_size(ds::dimensions<i32> size)
    {
        i32 result = SDL3::SDL_SetWindowMaximumSize(m_sdl_window, size.width, size.height);
        sdl_assert(result == 0, "failed to set max size");
        return result == 0;
    }

    SDL3::SDL_WindowFlags window::get_flags() const
    {
        return window::flag::type(SDL3::SDL_GetWindowFlags(m_sdl_window));
    }

    bool window::is_valid() const
    {
        return this->sdl_handle() != nullptr;
    }

    std::shared_ptr<sdl::renderer> window::renderer() const
    {
        return m_renderer;
    }

    SDL3::SDL_Window* window::sdl_handle() const
    {
        return m_sdl_window;
    }

    ds::dimensions<i32> window::get_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowSize(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to set size");
        return size;
    }

    ds::dimensions<i32> window::get_render_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowSizeInPixels(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to set render size");
        return size;
    }

    std::string window::get_title() const
    {
        return std::string{ SDL3::SDL_GetWindowTitle(m_sdl_window) };
    }

    ds::point<i32> window::get_position() const
    {
        ds::point<i32> pos{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowPosition(m_sdl_window, &pos.x, &pos.y);
        sdl_assert(result == 0, "failed to get pos");
        return pos;
    }

    ds::dimensions<i32> window::get_min_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowMinimumSize(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to get min size");
        return size;
    }

    ds::dimensions<i32> window::get_max_size() const
    {
        ds::dimensions<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowMaximumSize(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to get max size");
        return size;
    }

    bool window::get_grab() const
    {
        bool result = sdl::boolean(true) == SDL3::SDL_GetWindowGrab(m_sdl_window);
        sdl_assert(result, "failed to get window grab");
        return result;
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
