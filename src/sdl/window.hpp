#pragma once

namespace SDL3
{
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
}

#include <string>
#include <utility>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"

namespace rl::sdl
{
    class window
    {
    public:
        window(std::string title = "SDL3", ds::dimensions<i32> pos = { 1920, 1080 }, u32 flags = 0)
            : m_handle(SDL3::SDL_CreateWindow(title.data(), pos.width, pos.height, flags))
        {
        }

        window(std::string title, ds::rect<i32> bounds = { 0, 0, 1024, 768 }, u32 flags = 0)
            : m_handle(SDL3::SDL_CreateWindowWithPosition(title.data(),
                                                          bounds.pt.x,
                                                          bounds.pt.y,
                                                          bounds.size.width,
                                                          bounds.size.height,
                                                          flags))

        {
        }

        ~window()
        {
            if (m_handle != nullptr)
            {
                SDL3::SDL_DestroyWindow(m_handle);
                m_handle = nullptr;
            }
        }

        window(sdl::window&& other)
            : m_handle{ other.m_handle }
        {
            other.m_handle = nullptr;
        }

        const window& operator=(sdl::window&& other)
        {
            if (m_handle != nullptr)
            {
                SDL3::SDL_DestroyWindow(m_handle);
                m_handle = nullptr;
            }

            std::swap(m_handle, other.m_handle);
            return *this;
        }

        SDL3::SDL_Window* get_handle() const
        {
            return m_handle;
        }

        ds::dimensions<i32> get_size() const
        {
            ds::dimensions<i32> size{ 0, 0 };
            SDL3::SDL_GetWindowSize(m_handle, &size.width, &size.height);
            return size;
        }

        ds::dimensions<i32> get_render_size() const
        {
            ds::dimensions<i32> size{ 0, 0 };
            SDL3::SDL_GetWindowSizeInPixels(m_handle, &size.width, &size.height);
            return size;
        }

        const window& set_title(const std::string& title)
        {
            SDL3::SDL_SetWindowTitle(m_handle, title.c_str());
            return *this;
        }

        std::string get_title() const
        {
            return std::string{ SDL3::SDL_GetWindowTitle(m_handle) };
        }

        const window& maximize()
        {
            SDL3::SDL_MaximizeWindow(m_handle);
            return *this;
        }

        const window& minimize()
        {
            SDL3::SDL_MinimizeWindow(m_handle);
            return *this;
        }

        const window& hide()
        {
            SDL3::SDL_HideWindow(m_handle);
            return *this;
        }

        const window& restore()
        {
            SDL3::SDL_RestoreWindow(m_handle);
            return *this;
        }

        const window& raise()
        {
            SDL3::SDL_RaiseWindow(m_handle);
            return *this;
        }

        const window& show()
        {
            SDL3::SDL_ShowWindow(m_handle);
            return *this;
        }

        const window& set_fullscreen(bool fullscreen)
        {
            i32 result{ SDL3::SDL_SetWindowFullscreen(m_handle,
                                                      fullscreen ? SDL3::SDL_TRUE : SDL3::SDL_FALSE) };
            runtime_assert(result == 0, "Failed to set window to fullscreen");
            return *this;
        }

        const window& set_size(const ds::dimensions<i32>& size)
        {
            i32 result{ SDL3::SDL_SetWindowSize(m_handle, size.width, size.height) };
            runtime_assert(result == 0, "failed to set window size");
            return *this;
        }

        ds::point<i32> get_position() const
        {
            ds::point<i32> pos{ 0, 0 };
            SDL3::SDL_GetWindowPosition(m_handle, &pos.x, &pos.y);
            return pos;
        }

        const window& set_position(ds::point<i32> pos)
        {
            SDL3::SDL_SetWindowPosition(m_handle, pos.x, pos.y);
            return *this;
        }

        ds::dimensions<i32> get_min_size() const
        {
            ds::dimensions<i32> size{ 0, 0 };
            SDL3::SDL_GetWindowMinimumSize(m_handle, &size.width, &size.height);
            return size;
        }

        ds::dimensions<i32> get_max_size() const
        {
            ds::dimensions<i32> size{ 0, 0 };
            SDL3::SDL_GetWindowMaximumSize(m_handle, &size.width, &size.height);
            return size;
        }

        const window& set_min_size(ds::dimensions<i32> size)
        {
            SDL3::SDL_SetWindowMinimumSize(m_handle, size.width, size.height);
            return *this;
        }

        const window& set_max_size(ds::dimensions<i32>& size)
        {
            SDL3::SDL_SetWindowMaximumSize(m_handle, size.width, size.height);
            return *this;
        }

        bool get_grab() const
        {
            bool grab{ SDL3::SDL_GetWindowGrab(m_handle) == SDL3::SDL_TRUE };
            return grab;
        }

        const window& set_grab(bool grabbed)
        {
            SDL3::SDL_SetWindowGrab(m_handle, grabbed ? SDL3::SDL_TRUE : SDL3::SDL_FALSE);
            return *this;
        }

        SDL3::SDL_DisplayID get_display() const
        {
            SDL3::SDL_DisplayID id{ SDL3::SDL_GetDisplayForWindow(m_handle) };
            runtime_assert(id < 0, "failed to set window display idx");
            return id;
        }

        SDL3::SDL_DisplayMode&& get_display_mode() const
        {
            const SDL3::SDL_DisplayMode* mode{ SDL3::SDL_GetWindowFullscreenMode(m_handle) };
            runtime_assert(mode == nullptr, "failed to get window display mode");

            SDL3::SDL_DisplayMode ret{};
            SDL_memcpy(&ret, mode, sizeof(ret));
            return std::move(ret);
        }

        u32 get_flags() const
        {
            return SDL3::SDL_GetWindowFlags(m_handle);
        }

        const window& set_bordered(bool bordered)
        {
            SDL3::SDL_SetWindowBordered(m_handle, bordered ? SDL3::SDL_TRUE : SDL3::SDL_FALSE);
            return *this;
        }

        const window& set_opacity(float opacity)
        {
            i32 result{ SDL3::SDL_SetWindowOpacity(m_handle, opacity) };
            runtime_assert(result != 0, "failed to set window opacity");
            return *this;
        }

        f32 get_opacity() const
        {
            f32 opacity{ 0.0f };
            i32 result{ SDL3::SDL_GetWindowOpacity(m_handle, &opacity) };
            runtime_assert(result == -1, "failed to get window opacity");
            return opacity;
        }

        const window& set_resizable(bool resizable)
        {
            SDL3::SDL_SetWindowResizable(m_handle, resizable ? SDL3::SDL_TRUE : SDL3::SDL_FALSE);
            return *this;
        }

    private:
        SDL3::SDL_Window* m_handle{};
    };
}
