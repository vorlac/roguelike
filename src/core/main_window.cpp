#include <glad/gl.h>

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "core/assert.hpp"
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/popup.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "graphics/nvg_renderer.hpp"
#include "graphics/vg/nanovg.hpp"
#include "sdl/defs.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"
#include "utils/options.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {

    MainWindow::MainWindow(std::string title, const ds::dims<i32>& dims,
                           const MainWindow::Properties flags)
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
        m_window_rect = ds::rect<i32>{
            m_sdl_window ? this->get_position() : ds::point<i32>::null(),
            dims,
        };
        m_gl_renderer = std::make_unique<OpenGLRenderer>(*this);

        sdl_assert(m_sdl_window != nullptr, "failed to create SDL_Window");
        sdl_assert(m_gl_renderer != nullptr, "failed to create rl::OpenGLRenderer");

        this->get_display();
        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> render_size{ this->get_render_size() };

        SDL3::SDL_GL_SetSwapInterval(0);

        m_vg_renderer = std::make_unique<rl::NVGRenderer>();

        m_gui_canvas = new ui::Canvas{
            m_window_rect,
            m_mouse,
            m_keyboard,
            m_vg_renderer,
        };
    }

    MainWindow::~MainWindow()
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }
    }

    MainWindow& MainWindow::operator=(MainWindow&& other) noexcept
    {
        if (m_sdl_window != nullptr)
        {
            SDL3::SDL_DestroyWindow(m_sdl_window);
            m_sdl_window = nullptr;
        }

        std::swap(m_sdl_window, other.m_sdl_window);
        m_gl_renderer = std::move(other.m_gl_renderer);
        m_properties = std::move(other.m_properties);

        return *this;
    }

    bool MainWindow::maximize()
    {
        const i32 result{ SDL3::SDL_MaximizeWindow(m_sdl_window) };
        sdl_assert(result == 0, "failed to maximize");
        return result == 0;
    }

    bool MainWindow::minimize()
    {
        const i32 result{ SDL3::SDL_MinimizeWindow(m_sdl_window) };
        sdl_assert(result == 0, "failed to minimize");
        return result == 0;
    }

    bool MainWindow::hide()
    {
        const i32 result{ SDL3::SDL_HideWindow(m_sdl_window) };
        sdl_assert(result == 0, "failed hiding");
        return result == 0;
    }

    bool MainWindow::restore()
    {
        const i32 result{ SDL3::SDL_RestoreWindow(m_sdl_window) };
        sdl_assert(result == 0, "failed restoring");
        return result == 0;
    }

    bool MainWindow::raise()
    {
        const i32 result{ SDL3::SDL_RaiseWindow(m_sdl_window) };
        sdl_assert(result == 0, "failed raising");
        return result == 0;
    }

    bool MainWindow::show()
    {
        const i32 result{ SDL3::SDL_ShowWindow(m_sdl_window) };
        sdl_assert(result == 0, "failed to show");
        return result == 0;
    }

    bool MainWindow::set_vsync(bool enabled)
    {
        const i32 result{ SDL3::SDL_GL_SetSwapInterval(enabled ? 1 : 0) };
        sdl_assert(result == 0, "failed to set vsync (enabled:{})", enabled);
        m_vsync = enabled;
        return result == 0;
    }

    bool MainWindow::set_grab(const bool grabbed)
    {
        const i32 result{ SDL3::SDL_SetWindowGrab(m_sdl_window, grabbed) };
        sdl_assert(result == 0, "failed to set grab");
        return result == 0;
    }

    bool MainWindow::set_bordered(const bool bordered)
    {
        const i32 result{ SDL3::SDL_SetWindowBordered(m_sdl_window, bordered) };
        sdl_assert(result == 0, "failed to set bordered");
        return result == 0;
    }

    bool MainWindow::set_resizable(const bool resizable)
    {
        const i32 result{ SDL3::SDL_SetWindowResizable(m_sdl_window, resizable) };
        sdl_assert(result == 0, "failed to set resizeable");
        return result == 0;
    }

    bool MainWindow::set_fullscreen(const bool fullscreen)
    {
        const i32 result{ SDL3::SDL_SetWindowFullscreen(m_sdl_window, fullscreen) };
        runtime_assert(result == 0, "Failed to set window to fullscreen");
        return result == 0;
    }

    bool MainWindow::set_opacity(const float opacity)
    {
        const i32 result{ SDL3::SDL_SetWindowOpacity(m_sdl_window, opacity) };
        runtime_assert(result != 0, "failed to set window opacity");
        return result == 0;
    }

    bool MainWindow::set_title(std::string title)
    {
        m_title = title;
        const i32 result{ SDL3::SDL_SetWindowTitle(m_sdl_window, title.c_str()) };
        sdl_assert(result == 0, "failed to set title");
        return result == 0;
    }

    bool MainWindow::set_position(ds::point<i32> pos)
    {
        const i32 result{ SDL3::SDL_SetWindowPosition(m_sdl_window, pos.x, pos.y) };
        sdl_assert(result == 0, "failed to set position");
        m_window_rect.pt = pos;
        return result == 0;
    }

    bool MainWindow::set_size(ds::dims<i32> size)
    {
        const i32 result{ SDL3::SDL_SetWindowSize(m_sdl_window, size.width, size.height) };
        runtime_assert(result == 0, "failed to set size");
        m_window_rect.size = size;
        m_gui_canvas->set_size(ds::dims<f32>{
            static_cast<f32>(size.width),
            static_cast<f32>(size.height),
        });
        return result == 0;
    }

    bool MainWindow::set_min_size(ds::dims<i32> size)
    {
        const i32 result{ SDL3::SDL_SetWindowMinimumSize(m_sdl_window, size.width, size.height) };
        sdl_assert(result == 0, "failed to set min size");
        return result == 0;
    }

    bool MainWindow::set_max_size(ds::dims<i32> size)
    {
        const i32 result{ SDL3::SDL_SetWindowMaximumSize(m_sdl_window, size.width, size.height) };
        sdl_assert(result == 0, "failed to set max size");
        return result == 0;
    }

    MainWindow::Properties::Flags MainWindow::get_flags() const
    {
        return Properties::Flags(SDL3::SDL_GetWindowFlags(m_sdl_window));
    }

    bool MainWindow::is_valid() const
    {
        return this->sdl_handle() != nullptr;
    }

    const std::unique_ptr<OpenGLRenderer>& MainWindow::glrenderer() const
    {
        return m_gl_renderer;
    }

    const std::unique_ptr<NVGRenderer>& MainWindow::vgrenderer() const
    {
        return m_vg_renderer;
    }

    SDL3::SDL_Window* MainWindow::sdl_handle() const
    {
        return m_sdl_window;
    }

    const Keyboard& MainWindow::keyboard() const
    {
        return m_keyboard;
    }

    const Mouse& MainWindow::mouse() const
    {
        return m_mouse;
    }

    ui::Canvas* MainWindow::gui() const
    {
        return m_gui_canvas;
    }

    std::string MainWindow::get_title()
    {
        m_title = std::string{ SDL3::SDL_GetWindowTitle(m_sdl_window) };
        return m_title;
    }

    ds::point<i32> MainWindow::get_position() const
    {
        ds::point<i32> pos{ 0, 0 };
        i32 result{ SDL3::SDL_GetWindowPosition(m_sdl_window, &pos.x, &pos.y) };
        sdl_assert(result == 0, "failed to get pos");
        return pos;
    }

    ds::dims<i32> MainWindow::get_min_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result{ SDL3::SDL_GetWindowMinimumSize(m_sdl_window, &size.width, &size.height) };
        sdl_assert(result == 0, "failed to get min size");
        return size;
    }

    ds::dims<i32> MainWindow::get_max_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result{ SDL3::SDL_GetWindowMaximumSize(m_sdl_window, &size.width, &size.height) };
        sdl_assert(result == 0, "failed to get max size");
        return size;
    }

    bool MainWindow::input_grabbed() const
    {
        return SDL3::SDL_GetWindowGrab(m_sdl_window);
    }

    DisplayID MainWindow::get_display()
    {
        m_display_id = SDL3::SDL_GetDisplayForWindow(m_sdl_window);
        runtime_assert(m_display_id != 0, "failed to set window display idx");
        return m_display_id;
    }

    SDL3::SDL_DisplayMode MainWindow::get_display_mode() const
    {
        SDL3::SDL_DisplayMode ret{};
        const SDL3::SDL_DisplayMode* mode{ SDL3::SDL_GetWindowFullscreenMode(m_sdl_window) };
        runtime_assert(mode == nullptr, "failed to get window display mode");

        if (mode != nullptr)
            std::memcpy(&ret, mode, sizeof(ret));

        return ret;
    }

    f32 MainWindow::get_opacity() const
    {
        f32 opacity{ 0.0f };
        i32 result{ SDL3::SDL_GetWindowOpacity(m_sdl_window, &opacity) };
        runtime_assert(result == -1, "failed to get window opacity");
        return opacity;
    }

    ds::dims<i32> MainWindow::get_size()
    {
        i32 result{ SDL3::SDL_GetWindowSize(m_sdl_window, &m_window_rect.size.width,
                                            &m_window_rect.size.height) };
        sdl_assert(result == 0, "failed to set size");
        return m_window_rect.size;
    }

    ds::dims<i32> MainWindow::get_render_size()
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

    bool MainWindow::set_opengl_attribute(OpenGL::Attribute attr, auto val)
    {
        i32 result{ SDL3::SDL_GL_SetAttribute(MainWindow::OpenGL::type(attr), val) };
        sdl_assert(result == 0, "failed to set OpenGL attribute");
        return result == 0;
    }

    bool MainWindow::clear()
    {
        return m_gl_renderer->clear();
    }

    bool MainWindow::render_start()
    {
        bool ret{ true };
        ret &= this->swap_buffers();
        ret &= m_gui_canvas->draw_setup();
        return ret;
    }

    bool MainWindow::render_end()
    {
        return this->swap_buffers();
    }

    bool MainWindow::swap_buffers()
    {
        return m_gl_renderer->swap_buffers(*this);
    }

    bool MainWindow::render()
    {
        const i32 result{ SDL3::SDL_GL_MakeCurrent(m_sdl_window, m_gl_renderer->gl_context()) };
        sdl_assert(result == 0, "failed to make context current");

        m_gui_canvas->draw_all();
        m_gl_renderer->clear();
        return result == 0;
    }

    void MainWindow::mouse_entered_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        m_gui_canvas->on_mouse_entered(m_mouse);
    }

    void MainWindow::mouse_exited_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        m_gui_canvas->on_mouse_exited(m_mouse);
    }

    void MainWindow::mouse_moved_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_trace(log_level::trace);

        m_mouse.process_motion(e.motion);
        // update button states from pressed to held if the
        // button that was pressed last frame is still dowwn
        if (m_mouse.is_button_pressed(Mouse::Button::Left))
            this->mouse_button_pressed_event_callback(e);

        m_gui_canvas->on_mouse_move(m_mouse, m_keyboard);
    }

    void MainWindow::mouse_wheel_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        m_mouse.process_wheel(e.wheel);
        m_gui_canvas->on_mouse_scroll(m_mouse, m_keyboard);
    }

    void MainWindow::mouse_button_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const Mouse::Button::ID button_pressed{ e.button.button };
        m_mouse.process_button_down(button_pressed);
        m_gui_canvas->on_mouse_button_pressed(m_mouse, m_keyboard);
    }

    void MainWindow::mouse_button_released_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const Mouse::Button::ID button_released{ e.button.button };
        m_mouse.process_button_up(button_released);
        m_gui_canvas->on_mouse_button_released(m_mouse, m_keyboard);
    }

    void MainWindow::keyboard_key_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const Keyboard::Scancode::ID pressed_button{ e.key.keysym.scancode };
        m_keyboard.process_button_down(pressed_button);
        m_gui_canvas->on_key_pressed(m_keyboard);
    }

    void MainWindow::keyboard_key_released_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const Keyboard::Scancode::ID released_button{ e.key.keysym.scancode };
        m_keyboard.process_button_up(released_button);
        m_gui_canvas->on_key_pressed(m_keyboard);
    }

    void MainWindow::keyboard_char_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        switch (e.type)
        {
            case Keyboard::Event::TextInput:
                m_keyboard.process_text_input(e.text.text);
                break;
            case Keyboard::Event::TextEditing:
                m_keyboard.process_text_editing(e.edit.text, e.edit.start, e.edit.length);
                break;
        }

        m_gui_canvas->on_character_input(m_keyboard);
    }

    void MainWindow::window_resized_event_callback(const SDL3::SDL_Event& e)
    {
        ds::dims<i32> window_size{ this->get_size() };

        scoped_log("size={}", window_size);
        ds::dims<i32> framebuf_size{ this->get_render_size() };
        ds::dims<f32> render_size{ window_size / m_pixel_ratio };
        runtime_assert(framebuf_size.area() > 0.0f && window_size.area() > 0.0f,
                       "invalid window size/location");

        m_framebuf_size = framebuf_size;
        m_window_rect.size = static_cast<ds::dims<i32>>(render_size);
        m_gl_renderer->set_viewport(ds::rect<i32>{
            ds::point<i32>{ 0, 0 },
            m_window_rect.size,
        });

        m_gui_canvas->on_resized(render_size);
    }

    void MainWindow::window_moved_event_callback(const SDL3::SDL_Event& e)
    {
        ds::point<i32> window_pos{ this->get_position() };

        scoped_log("mod_pos={}", window_pos);
        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> framebuf_size{ this->get_render_size() };
        runtime_assert(framebuf_size.area() > 0.0f && window_size.area() > 0.0f,
                       "invalid window size/location");

        m_framebuf_size = framebuf_size;
        m_window_rect.pt = window_pos;
        m_window_rect.size = ds::dims<i32>{
            static_cast<i32>(window_size.width / m_pixel_ratio),
            static_cast<i32>(window_size.height / m_pixel_ratio),
        };
    }

    bool MainWindow::window_pixel_size_changed_event_callback(const SDL3::SDL_Event& e)
    {
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        const ds::vector2<i32> pixel_size{
            window_event.data1,
            window_event.data2,
        };

        scoped_log();
        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);
        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);
        return m_pixel_ratio > 0.0f && m_pixel_density > 0.0f;
    }

    void MainWindow::window_focus_gained_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        m_gui_canvas->on_focus_gained();
    }

    void MainWindow::window_focus_lost_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        m_gui_canvas->on_focus_lost();
    }

    bool MainWindow::window_shown_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_occluded_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_hidden_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_exposed_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_minimized_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_maximized_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_restored_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_close_requested_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_take_focus_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_hit_test_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_icc_profile_changed_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_display_changed_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_display_scale_changed_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }

    bool MainWindow::window_destroyed_event_callback(const SDL3::SDL_Event& e)
    {
        scoped_log();
        const MainWindow::Event::Data& window_event{ e.window };
        const WindowID id{ window_event.windowID };
        return true;
    }
}
