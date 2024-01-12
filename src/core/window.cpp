#include <glad/gl.h>

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include <nanovg.h>

#include "core/assert.hpp"
#include "core/renderer.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/popup.hpp"
#include "core/ui/screen.hpp"
#include "core/window.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "sdl/utils.hpp"
#include "utils/io.hpp"
#include "utils/numeric.hpp"
#include "utils/options.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {

    Window::Window(std::string title, const ds::dims<i32>& dims, Window::Properties flags)
    {
        this->set_opengl_attribute(OpenGL::Attribute::AcceleratedVisual, 1);
        this->set_opengl_attribute(OpenGL::Attribute::ContextMajorVersion, 4);
        this->set_opengl_attribute(OpenGL::Attribute::ContextMinorVersion, 6);
        this->set_opengl_attribute(OpenGL::Attribute::DepthSize, 24);
        this->set_opengl_attribute(OpenGL::Attribute::StencilSize, 8);
        this->set_opengl_attribute(OpenGL::Attribute::RetainedBacking, 0);
        this->set_opengl_attribute(OpenGL::Attribute::RedSize, 8);
        this->set_opengl_attribute(OpenGL::Attribute::GreenSize, 8);
        this->set_opengl_attribute(OpenGL::Attribute::BlueSize, 8);
        this->set_opengl_attribute(OpenGL::Attribute::AlphaSize, 8);
        this->set_opengl_attribute(OpenGL::Attribute::Doublebuffer, 1);
        this->set_opengl_attribute(OpenGL::Attribute::ContextProfileMask, OpenGL::Profile::Core);
        this->set_opengl_attribute(OpenGL::Attribute::ContextFlags,
                                   OpenGL::ContextFlag::ForwardCompatible);

        m_properties = flags;
        m_sdl_window = SDL3::SDL_CreateWindow(title.data(), dims.width, dims.height, m_properties);
        m_window_rect = ds::rect{
            m_sdl_window ? this->get_position() : ds::point<i32>::null(),
            dims,
        };
        m_renderer = std::make_unique<Renderer>(*this, Renderer::DEFAULT_PROPERTY_FLAGS);

        sdl_assert(m_sdl_window != nullptr, "failed to create SDL_Window");
        sdl_assert(m_renderer != nullptr, "failed to create sdl::Renderer");

        this->get_display();
        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> render_size{ this->get_render_size() };

        SDL3::SDL_GL_SetSwapInterval(1);

        NVGcontext* nvg_context{ m_renderer->nvg_context() };
        m_screen = new ui::Screen{ nvg_context, window_size, m_mouse, m_keyboard };
    }

    Window::~Window()
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }
    }

    const Window& Window::operator=(Window&& other) noexcept
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }

        std::swap(m_sdl_window, other.m_sdl_window);
        m_renderer = std::move(other.m_renderer);
        m_properties = std::move(other.m_properties);

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

    bool Window::set_vsync(bool enabled)
    {
        i32 result = SDL3::SDL_GL_SetSwapInterval(enabled ? 1 : 0);
        sdl_assert(result == 0, "failed to set vsync (enabled:{})", enabled);
        m_vsync = enabled;
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
        m_title = title;
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
        i32 result = SDL3::SDL_SetWindowSize(m_sdl_window, size.width, size.height);
        runtime_assert(result == 0, "failed to set size");
        m_window_rect.size = size;
        m_screen->set_size(size);
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

    const std::unique_ptr<Renderer>& Window::renderer() const
    {
        return m_renderer;
    }

    SDL3::SDL_Window* Window::sdl_handle() const
    {
        return m_sdl_window;
    }

    NVGcontext* Window::nvg_context() const
    {
        // Return a pointer to the window renderer's underlying NanoVG draw context
        return m_renderer->nvg_context();
    }

    const Keyboard& Window::keyboard() const
    {
        return m_keyboard;
    }

    const Mouse& Window::mouse() const
    {
        return m_mouse;
    }

    ui::Screen* Window::gui() const
    {
        return m_screen;
    }

    std::string Window::get_title()
    {
        m_title = std::string{ SDL3::SDL_GetWindowTitle(m_sdl_window) };
        return m_title;
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
        i32 result = SDL3::SDL_GetWindowGrab(m_sdl_window);
        sdl_assert(result == 1, "failed to get window grab");
        return result == 1;
    }

    DisplayID Window::get_display()
    {
        m_display_id = SDL3::SDL_GetDisplayForWindow(m_sdl_window);
        runtime_assert(m_display_id != 0, "failed to set window display idx");
        return m_display_id;
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

    ds::dims<i32> Window::get_size()
    {
        i32 result = SDL3::SDL_GetWindowSize(m_sdl_window, &m_window_rect.size.width,
                                             &m_window_rect.size.height);
        sdl_assert(result == 0, "failed to set size");
        return m_window_rect.size;
    }

    ds::dims<i32> Window::get_render_size()
    {
        i32 result{ SDL3::SDL_GetWindowSizeInPixels(m_sdl_window, &m_framebuf_size.width,
                                                    &m_framebuf_size.height) };
        sdl_assert(result == 0, "failed to set render size");
        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);
        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);
        return m_framebuf_size;
    }

    bool Window::set_opengl_attribute(OpenGL::Attribute attr, auto val)
    {
        i32 result{ SDL3::SDL_GL_SetAttribute(Window::OpenGL::type(attr), val) };
        sdl_assert(result == 0, "failed to set OpenGL attribute");
        return result == 0;
    }

    bool Window::on_pixel_size_changed(const WindowID id, ds::dims<i32> pixel_size)
    {
        bool ret{ true };

        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);
        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);

        if constexpr (io::logging::window_events)
            log::info("window::on_pixel_size_changed [id:{}] => {}", id, pixel_size);

        return ret;
    }

    bool Window::on_occluded(const WindowID id)
    {
        bool ret = true;
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_occluded [id:{}]", id);
        return ret;
    }

    bool Window::clear()
    {
        return m_renderer->clear();
    }

    bool Window::render_start()
    {
        bool ret{ true };
        ret &= this->swap_buffers();
        ret &= m_screen->draw_setup();

        return ret;
    }

    bool Window::render_end()
    {
        return this->swap_buffers();
    }

    bool Window::swap_buffers()
    {
        return m_renderer->swap_buffers(*this);
    }

    bool Window::render()
    {
        i32 result{ SDL3::SDL_GL_MakeCurrent(m_sdl_window, m_renderer->gl_context()) };
        sdl_assert(result == 0, "failed to make context current");

        m_screen->draw_all();
        // TODO: remove / move
        m_renderer->clear();

        return result == 0;
    }

    void Window::mouse_entered_event_callback(const SDL3::SDL_Event& e)
    {
        m_mouse.process_motion(e.motion);
        m_screen->on_mouse_entered(m_mouse);
    }

    void Window::mouse_exited_event_callback(const SDL3::SDL_Event& e)
    {
        m_mouse.process_motion(e.motion);
        m_screen->on_mouse_exited(m_mouse);
    }

    void Window::mouse_moved_event_callback(const SDL3::SDL_Event& e)
    {
        m_mouse.process_motion(e.motion);
        m_screen->on_mouse_move(m_mouse, m_keyboard);
    }

    void Window::mouse_wheel_event_callback(const SDL3::SDL_Event& e)
    {
        m_mouse.process_wheel(e.wheel);
        m_screen->on_mouse_scroll(m_mouse, m_keyboard);
    }

    void Window::mouse_button_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        const auto button_pressed{ e.button.button };
        m_mouse.process_button_down(button_pressed);
        m_screen->on_mouse_button_pressed(m_mouse, m_keyboard);
    }

    void Window::mouse_button_released_event_callback(const SDL3::SDL_Event& e)
    {
        const auto button_released{ e.button.button };
        m_mouse.process_button_down(button_released);
        m_screen->on_mouse_button_released(m_mouse, m_keyboard);
    }

    void Window::keyboard_key_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        const auto pressed_button{ e.key.keysym.scancode };
        m_keyboard.process_button_down(pressed_button);
        m_screen->on_key_pressed(m_keyboard);
    }

    void Window::keyboard_key_released_event_callback(const SDL3::SDL_Event& e)
    {
        const auto released_button(e.key.keysym.scancode);
        m_keyboard.process_button_up(released_button);
        m_screen->on_key_pressed(m_keyboard);
    }

    void Window::keyboard_char_event_callback(const SDL3::SDL_Event& e)
    {
        switch (e.type)
        {
            case Keyboard::Event::TextInput:
                m_keyboard.process_text_input(e.text.text);
                break;
            case Keyboard::Event::TextEditing:
                m_keyboard.process_text_editing(e.edit.text, e.edit.start, e.edit.length);
                break;
        }
        m_screen->on_character_input(m_keyboard);
    }

    void Window::window_resized_event_callback(const SDL3::SDL_Event& e)
    {
        // TODO: just use the values from the resize event struct
        // and the existing m_pixel_ratio sizes instead of calling
        // the SDL APIs for the dimensions instead?

        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> framebuf_size{ this->get_render_size() };
        if (framebuf_size.area() == 0 || window_size.area() == 0)
            return;

        m_framebuf_size = framebuf_size;
        m_window_rect.size = ds::dims<i32>{
            static_cast<i32>(window_size.width / m_pixel_ratio),
            static_cast<i32>(window_size.height / m_pixel_ratio),
        };

        m_renderer->set_viewport(std::forward<ds::rect<i32>>({
            ds::point<i32>{ 0, 0 },
            m_window_rect.size,
        }));

        m_screen->on_resized(std::forward<ds::dims<i32>>(m_window_rect.size));
    }

    void Window::window_focus_gained_event_callback(const SDL3::SDL_Event& e)
    {
        m_screen->on_focus_gained();
    }

    void Window::window_focus_lost_event_callback(const SDL3::SDL_Event& e)
    {
        m_screen->on_focus_lost();
    }
}
