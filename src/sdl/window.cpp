#include <glad/gl.h>

#include <memory>
#include <utility>

#include <fmt/format.h>

#include "core/numeric.hpp"
#include "core/options.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer_opengl.hpp"
#include "sdl/utils.hpp"
#include "utils/assert.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    Window::Window()
    {
        // SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_ACCELERATED_VISUAL, 1);
        // SDL3::SDL_GL_SetAttribute(
        //    SDL3::SDL_GL_CONTEXT_FLAGS,
        //    SDL3::SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG |
        //    SDL3::SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_DOUBLEBUFFER, 1);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_PROFILE_MASK,
                                  SDL3::SDL_GL_CONTEXT_PROFILE_CORE);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MINOR_VERSION, 3);
    }

    Window::Window(std::string title, const ds::dims<i32>& dims, Window::Properties flags)
        : Window()
    {
        m_properties = flags;
        m_sdl_window = SDL3::SDL_CreateWindow(title.data(), dims.width, dims.height, m_properties);
        m_window_rect = { m_sdl_window ? this->get_position() : ds::point<i32>::null(), dims };
        m_renderer = std::shared_ptr<sdl::RendererGL>(
            new sdl::RendererGL(*this, RendererGL::DEFAULT_PROPERTY_FLAGS));
        sdl_assert(m_sdl_window != nullptr, "failed to create SDL_Window");
        sdl_assert(m_renderer != nullptr, "failed to create sdl::Renderer");
    }

    Window::~Window()
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }
    }

    const Window& Window::operator=(sdl::Window&& other) noexcept
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }

        std::swap(m_sdl_window, other.m_sdl_window);
        m_renderer = std::move(other.m_renderer);
        m_properties = std::move(other.m_properties);
        m_window_rect = std::move(other.m_window_rect);

        return *this;
    }

    bool Window::maximize()
    {
        i32 result = SDL3::SDL_MaximizeWindow(m_sdl_window);
        sdl_assert(result == 0, "failed to maximize");
        return result == 0;
    }

    bool Window::minimize()
    {
        i32 result = SDL3::SDL_MinimizeWindow(m_sdl_window);
        sdl_assert(result == 0, "failed to minimize");
        return result == 0;
    }

    bool Window::hide()
    {
        i32 result = SDL3::SDL_HideWindow(m_sdl_window);
        sdl_assert(result == 0, "failed hiding");
        return result == 0;
    }

    bool Window::restore()
    {
        i32 result = SDL3::SDL_RestoreWindow(m_sdl_window);
        sdl_assert(result == 0, "failed restoring");
        return result == 0;
    }

    bool Window::raise()
    {
        i32 result = SDL3::SDL_RaiseWindow(m_sdl_window);
        sdl_assert(result == 0, "failed raising");
        return result == 0;
    }

    bool Window::show()
    {
        i32 result = SDL3::SDL_ShowWindow(m_sdl_window);
        sdl_assert(result == 0, "failed to show");
        return result == 0;
    }

    bool Window::set_grab(bool grabbed)
    {
        i32 result = SDL3::SDL_SetWindowGrab(m_sdl_window, grabbed);
        sdl_assert(result == 0, "failed to set grab");
        return result == 0;
    }

    bool Window::set_bordered(bool bordered)
    {
        i32 result = SDL3::SDL_SetWindowBordered(m_sdl_window, bordered);
        sdl_assert(result == 0, "failed to set bordered");
        return result == 0;
    }

    bool Window::set_resizable(bool resizable)
    {
        i32 result = SDL3::SDL_SetWindowResizable(m_sdl_window, resizable);
        sdl_assert(result == 0, "failed to set resizeable");
        return result == 0;
    }

    bool Window::set_fullscreen(bool fullscreen)
    {
        i32 result{ SDL3::SDL_SetWindowFullscreen(m_sdl_window, fullscreen) };
        runtime_assert(result == 0, "Failed to set window to fullscreen");
        return result == 0;
    }

    bool Window::set_opacity(float opacity)
    {
        i32 result = SDL3::SDL_SetWindowOpacity(m_sdl_window, opacity);
        runtime_assert(result != 0, "failed to set window opacity");
        return result == 0;
    }

    bool Window::set_title(std::string title)
    {
        i32 result = SDL3::SDL_SetWindowTitle(m_sdl_window, title.c_str());
        sdl_assert(result == 0, "failed to set title");
        return result == 0;
    }

    bool Window::set_position(ds::point<i32> pos)
    {
        i32 result = SDL3::SDL_SetWindowPosition(m_sdl_window, pos.x, pos.y);
        sdl_assert(result == 0, "failed to set position");
        m_window_rect.pt = pos;
        return result == 0;
    }

    bool Window::set_size(ds::dims<i32> size)
    {
        i32 result{ SDL3::SDL_SetWindowSize(m_sdl_window, size.width, size.height) };
        runtime_assert(result == 0, "failed to set size");
        m_window_rect.size = size;
        return result == 0;
    }

    bool Window::set_min_size(ds::dims<i32> size)
    {
        i32 result = SDL3::SDL_SetWindowMinimumSize(m_sdl_window, size.width, size.height);
        sdl_assert(result == 0, "failed to set min size");
        return result == 0;
    }

    bool Window::set_max_size(ds::dims<i32> size)
    {
        i32 result = SDL3::SDL_SetWindowMaximumSize(m_sdl_window, size.width, size.height);
        sdl_assert(result == 0, "failed to set max size");
        return result == 0;
    }

    SDL3::SDL_WindowFlags Window::get_flags() const
    {
        return Window::Properties(SDL3::SDL_GetWindowFlags(m_sdl_window));
    }

    bool Window::is_valid() const
    {
        return this->sdl_handle() != nullptr;
    }

    std::shared_ptr<sdl::RendererGL> Window::renderer() const
    {
        return m_renderer;
    }

    bool Window::swap_buffers()
    {
        return m_renderer->swap_buffers(*this);
    }

    SDL3::SDL_Window* Window::sdl_handle() const
    {
        return m_sdl_window;
    }

    ds::dims<i32> Window::get_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowSize(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to set size");
        return size;
    }

    ds::dims<i32> Window::get_render_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowSizeInPixels(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to set render size");
        return size;
    }

    std::string Window::get_title() const
    {
        return std::string{ SDL3::SDL_GetWindowTitle(m_sdl_window) };
    }

    ds::point<i32> Window::get_position() const
    {
        ds::point<i32> pos{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowPosition(m_sdl_window, &pos.x, &pos.y);
        sdl_assert(result == 0, "failed to get pos");
        return pos;
    }

    ds::dims<i32> Window::get_min_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowMinimumSize(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to get min size");
        return size;
    }

    ds::dims<i32> Window::get_max_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_GetWindowMaximumSize(m_sdl_window, &size.width, &size.height);
        sdl_assert(result == 0, "failed to get max size");
        return size;
    }

    bool Window::get_grab() const
    {
        bool result = sdl::boolean(true) == SDL3::SDL_GetWindowGrab(m_sdl_window);
        sdl_assert(result, "failed to get window grab");
        return result;
    }

    SDL3::SDL_DisplayID Window::get_display() const
    {
        SDL3::SDL_DisplayID id{ SDL3::SDL_GetDisplayForWindow(m_sdl_window) };
        runtime_assert(id < 0, "failed to set window display idx");
        return id;
    }

    SDL3::SDL_DisplayMode Window::get_display_mode() const
    {
        SDL3::SDL_DisplayMode ret{};
        const SDL3::SDL_DisplayMode* mode{ SDL3::SDL_GetWindowFullscreenMode(m_sdl_window) };
        runtime_assert(mode == nullptr, "failed to get window display mode");
        if (mode != nullptr)
            SDL3::SDL_memcpy(&ret, mode, sizeof(ret));
        return ret;
    }

    f32 Window::get_opacity() const
    {
        f32 opacity{ 0.0f };
        i32 result{ SDL3::SDL_GetWindowOpacity(m_sdl_window, &opacity) };
        runtime_assert(result == -1, "failed to get window opacity");
        return opacity;
    }

    bool Window::on_shown(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_shown [id:{}]", id);
        return ret;
    }

    bool Window::on_hidden(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_hidden [id:{}]", id);
        return ret;
    }

    bool Window::on_exposed(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_exposed [id:{}]", id);
        return ret;
    }

    bool Window::on_moved(sdl::WindowID id, ds::point<i32>&& pt)
    {
        if constexpr (io::logging::window_events)
        {
            ds::rect<i32> prev_rect{ m_window_rect };
            ds::rect<i32> new_rect{ pt, prev_rect.size };
            log::info("window::on_moved [id={}] : {} => {}", id, prev_rect, new_rect);
        }

        bool ret = m_window_rect.pt != pt;
        runtime_assert(ret, "window moved, but location unchanged");
        m_window_rect.pt = pt;

        return ret;
    }

    bool Window::on_resized(const WindowID id, ds::dims<i32>&& size)
    {
        if constexpr (io::logging::window_events)
        {
            ds::rect<i32> prev_rect{ m_window_rect };
            ds::rect<i32> new_rect{ prev_rect.pt, size };
            log::info("window::on_resized [id={}] : {} => {}", id, prev_rect, new_rect);
        }

        bool ret = m_window_rect.size != size;
        runtime_assert(ret, "window resized, but size unchanged");
        glViewport(0, 0, size.width, size.height);
        m_window_rect.size = size;

        return ret;
    }

    bool Window::on_pixel_size_changed(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_pixel_size_changed [id:{}]", id);
        return ret;
    }

    bool Window::on_minimized(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_minimized [id:{}]", id);
        return ret;
    }

    bool Window::on_maximized(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_maximized [id:{}]", id);
        return ret;
    }

    bool Window::on_restored(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_restored [id:{}]", id);
        return ret;
    }

    bool Window::on_mouse_enter(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_enter [id:{}]", id);
        return ret;
    }

    bool Window::on_mouse_leave(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_leave [id:{}]", id);
        return ret;
    }

    bool Window::on_kb_focus_gained(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_kb_focus_gained [id:{}]", id);
        return ret;
    }

    bool Window::on_kb_focus_lost(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_kb_focus_lost [id:{}]", id);
        return ret;
    }

    bool Window::on_close_requested(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_close_requested [id:{}]", id);
        return ret;
    }

    bool Window::on_take_focus(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_take_focus [id:{}]", id);
        return ret;
    }

    bool Window::on_hit_test(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_hit_test [id:{}]", id);
        return ret;
    }

    bool Window::on_icc_profile_changed(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_icc_profile_changed [id:{}]", id);
        return ret;
    }

    bool Window::on_display_changed(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_display_changed [id:{}]", id);
        return ret;
    }

    bool Window::on_display_scale_changed(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_display_scale_changed [id:{}]", id);
        return ret;
    }

    bool Window::on_occluded(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_occluded [id:{}]", id);
        return ret;
    }

    bool Window::on_destroyed(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_destroyed [id:{}]", id);
        return ret;
    }
}
