#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "core/assert.hpp"
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "gfx/nvg_renderer.hpp"
#include "ui/canvas.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"
#include "utils/sdl_defs.hpp"

namespace rl {
    MainWindow::MainWindow(const std::string& title, const ds::dims<i32>& dims,
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

        m_sdl_window = {
            SDL3::SDL_CreateWindow(title.data(), dims.width, dims.height, m_properties),
            SDL3::SDL_DestroyWindow,
        };

        m_window_id = this->get_window_id();
        m_window_rect = ds::rect{
            m_sdl_window ? this->get_position() : ds::point<i32>::null(),
            dims,
        };
        m_gl_renderer = std::make_unique<OpenGLRenderer>(*this);

        sdl_assert(m_sdl_window != nullptr, "failed to create SDL_Window");
        sdl_assert(m_gl_renderer != nullptr, "failed to create rl::OpenGLRenderer");

        this->get_display_id();

        SDL3::SDL_GL_SetSwapInterval(0);

        m_vg_renderer = std::make_unique<NVGRenderer>();
        m_gui_canvas = std::make_unique<ui::Canvas>(
            this, static_cast<ds::rect<f32>>(m_window_rect),
            m_mouse, m_keyboard, m_vg_renderer);
    }

    MainWindow::~MainWindow()
    {
    }

    MainWindow& MainWindow::operator=(MainWindow&& other) noexcept
    {
        if (m_sdl_window != nullptr) {
            SDL3::SDL_DestroyWindow(m_sdl_window.get());
            m_sdl_window = nullptr;
        }

        m_sdl_window.swap(other.m_sdl_window);
        m_gl_renderer = std::move(other.m_gl_renderer);
        m_properties = other.m_properties;

        return *this;
    }

    bool MainWindow::maximize() const
    {
        const i32 result{ SDL3::SDL_MaximizeWindow(m_sdl_window.get()) };
        sdl_assert(result == 0, "failed to maximize");
        return result == 0;
    }

    bool MainWindow::minimize() const
    {
        const i32 result{ SDL3::SDL_MinimizeWindow(m_sdl_window.get()) };
        sdl_assert(result == 0, "failed to minimize");
        return result == 0;
    }

    bool MainWindow::hide() const
    {
        const i32 result{ SDL3::SDL_HideWindow(m_sdl_window.get()) };
        sdl_assert(result == 0, "failed hiding");
        return result == 0;
    }

    bool MainWindow::restore() const
    {
        const i32 result{ SDL3::SDL_RestoreWindow(m_sdl_window.get()) };
        sdl_assert(result == 0, "failed restoring");
        return result == 0;
    }

    bool MainWindow::raise() const
    {
        const i32 result{ SDL3::SDL_RaiseWindow(m_sdl_window.get()) };
        sdl_assert(result == 0, "failed raising");
        return result == 0;
    }

    bool MainWindow::show() const
    {
        const i32 result{ SDL3::SDL_ShowWindow(m_sdl_window.get()) };
        sdl_assert(result == 0, "failed to show");
        return result == 0;
    }

    WindowID MainWindow::window_id() const
    {
        debug_assert(m_window_id != 0, "invalid window ID");
        return m_window_id;
    }

    WindowID MainWindow::get_window_id()
    {
        const WindowID result{ SDL3::SDL_GetWindowID(m_sdl_window.get()) };
        sdl_assert(result != 0, "failed to get window id");
        sdl_assert(m_window_id == 0 || result == m_window_id, "sdl window id mismatch");
        m_window_id = result;
        return m_window_id;
    }

    bool MainWindow::set_vsync(bool enabled)
    {
        const i32 result{ SDL3::SDL_GL_SetSwapInterval(enabled ? 1 : 0) };
        sdl_assert(result == 0, "failed to set vsync (enabled:{})", enabled);
        m_vsync = enabled;
        return result == 0;
    }

    bool MainWindow::set_kb_grab(const bool grabbed) const
    {
        const i32 result{ SDL3::SDL_SetWindowKeyboardGrab(m_sdl_window.get(), grabbed) };
        sdl_assert(result == 0, "failed to set kb grab");
        return result == 0;
    }

    bool MainWindow::set_mouse_grab(const bool grabbed) const
    {
        const i32 result{ SDL3::SDL_SetWindowMouseGrab(m_sdl_window.get(), grabbed) };
        sdl_assert(result == 0, "failed to set mouse grab");
        return result == 0;
    }

    bool MainWindow::set_bordered(const bool bordered) const
    {
        const i32 result{ SDL3::SDL_SetWindowBordered(m_sdl_window.get(), bordered) };
        sdl_assert(result == 0, "failed to set bordered");
        return result == 0;
    }

    bool MainWindow::set_resizable(const bool resizable) const
    {
        const i32 result{ SDL3::SDL_SetWindowResizable(m_sdl_window.get(), resizable) };
        sdl_assert(result == 0, "failed to set resizeable");
        return result == 0;
    }

    bool MainWindow::set_fullscreen(const bool fullscreen) const
    {
        const i32 result{ SDL3::SDL_SetWindowFullscreen(m_sdl_window.get(), fullscreen) };
        debug_assert(result == 0, "Failed to set window to fullscreen");
        return result == 0;
    }

    bool MainWindow::set_opacity(const float opacity) const
    {
        const i32 result{ SDL3::SDL_SetWindowOpacity(m_sdl_window.get(), opacity) };
        debug_assert(result != 0, "failed to set window opacity");
        return result == 0;
    }

    bool MainWindow::set_background([[maybe_unused]] const ds::color<u8>& background) const
    {
        debug_assert(false, "not implemented: MainWindow::background({})", background);
        return false;
    }

    bool MainWindow::set_title(const std::string& title)
    {
        m_title = title;
        const i32 result{ SDL3::SDL_SetWindowTitle(m_sdl_window.get(), title.c_str()) };
        sdl_assert(result == 0, "failed to set title");
        return result == 0;
    }

    bool MainWindow::set_modal([[maybe_unused]] bool modal) const
    {
        debug_assert("not implemented: MainWindow::set_modal({})", modal);
        return false;
    }

    bool MainWindow::set_position(const ds::point<i32>& pos)
    {
        const i32 result{ SDL3::SDL_SetWindowPosition(m_sdl_window.get(), pos.x, pos.y) };
        sdl_assert(result == 0, "failed to set position");
        m_window_rect.pt = pos;
        return result == 0;
    }

    bool MainWindow::set_size(const ds::dims<i32> size)
    {
        const i32 result{ SDL3::SDL_SetWindowSize(m_sdl_window.get(), size.width, size.height) };
        debug_assert(result == 0, "failed to set size");

        m_window_rect.size = size;
        const ds::rect<f32> canvas_rect{ m_gui_canvas->rect() };
        if (size != canvas_rect.size)
            m_gui_canvas->set_size(size);

        return result == 0;
    }

    bool MainWindow::set_min_size(const ds::dims<i32>& size) const
    {
        const i32 result{ SDL3::SDL_SetWindowMinimumSize(m_sdl_window.get(), size.width, size.height) };
        sdl_assert(result == 0, "failed to set min size");
        return result == 0;
    }

    bool MainWindow::set_max_size(const ds::dims<i32>& size) const
    {
        const i32 result{ SDL3::SDL_SetWindowMaximumSize(m_sdl_window.get(), size.width, size.height) };
        sdl_assert(result == 0, "failed to set max size");
        return result == 0;
    }

    MainWindow::Properties::Flags MainWindow::get_flags() const
    {
        return static_cast<Properties::Flags>(SDL3::SDL_GetWindowFlags(m_sdl_window.get()));
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
        return m_sdl_window.get();
    }

    const Keyboard& MainWindow::keyboard() const
    {
        return m_keyboard;
    }

    const Mouse& MainWindow::mouse() const
    {
        return m_mouse;
    }

    std::unique_ptr<ui::Canvas>& MainWindow::gui()
    {
        return m_gui_canvas;
    }

    std::string MainWindow::get_title()
    {
        m_title = std::string{ SDL3::SDL_GetWindowTitle(m_sdl_window.get()) };
        return m_title;
    }

    ds::point<i32> MainWindow::get_position() const
    {
        ds::point<i32> pos{ 0, 0 };
        [[maybe_unused]] const i32 result{ SDL3::SDL_GetWindowPosition(m_sdl_window.get(), &pos.x, &pos.y) };
        sdl_assert(result == 0, "failed to get pos");
        return pos;
    }

    ds::dims<i32> MainWindow::get_min_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        [[maybe_unused]] const i32 result{ SDL3::SDL_GetWindowMinimumSize(m_sdl_window.get(), &size.width, &size.height) };
        sdl_assert(result == 0, "failed to get min size");
        return size;
    }

    ds::dims<i32> MainWindow::get_max_size() const
    {
        ds::dims<i32> size{ 0, 0 };
        [[maybe_unused]] const i32 result{ SDL3::SDL_GetWindowMaximumSize(m_sdl_window.get(), &size.width, &size.height) };
        sdl_assert(result == 0, "failed to get max size");
        return size;
    }

    bool MainWindow::kb_grabbed() const
    {
        return SDL3::SDL_GetWindowKeyboardGrab(m_sdl_window.get());
    }

    bool MainWindow::mouse_grabbed() const
    {
        return SDL3::SDL_GetWindowMouseGrab(m_sdl_window.get());
    }

    DisplayID MainWindow::get_display_id()
    {
        m_display_id = SDL3::SDL_GetDisplayForWindow(m_sdl_window.get());
        debug_assert(m_display_id != 0, "failed to set window display idx");
        return m_display_id;
    }

    SDL3::SDL_DisplayMode MainWindow::get_display_mode() const
    {
        SDL3::SDL_DisplayMode ret{};
        const SDL3::SDL_DisplayMode* mode{ SDL3::SDL_GetWindowFullscreenMode(m_sdl_window.get()) };
        debug_assert(mode == nullptr, "failed to get window display mode");

        if (mode != nullptr)
            std::memcpy(&ret, mode, sizeof(ret));

        return ret;
    }

    f32 MainWindow::get_opacity() const
    {
        f32 opacity{ 0.0f };
        [[maybe_unused]] const i32 result{ SDL3::SDL_GetWindowOpacity(m_sdl_window.get(), &opacity) };
        debug_assert(result == -1, "failed to get window opacity");
        return opacity;
    }

    const ds::dims<i32>& MainWindow::get_size()
    {
        auto& size = m_window_rect.size;
        [[maybe_unused]] const i32 result{ SDL3::SDL_GetWindowSize(m_sdl_window.get(), &size.width, &size.height) };
        sdl_assert(result == 0, "failed to set size");
        return m_window_rect.size;
    }

    const ds::dims<i32>& MainWindow::get_render_size()
    {
        ds::dims<i32>& size{ m_framebuf_size };
        [[maybe_unused]] const i32 result{ SDL3::SDL_GetWindowSizeInPixels(m_sdl_window.get(), &size.width, &size.height) };
        sdl_assert(result == 0, "failed to set render size");
        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window.get());
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);
        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window.get());
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);
        return m_framebuf_size;
    }

    bool MainWindow::set_opengl_attribute(const OpenGL::Attribute attr, auto val)
    {
        const i32 result{ SDL3::SDL_GL_SetAttribute(static_cast<SDL3::SDL_GLattr>(attr), val) };
        sdl_assert(result == 0, "failed to set OpenGL attribute");
        return result == 0;
    }

    bool MainWindow::clear() const
    {
        return m_gl_renderer->clear();
    }

    bool MainWindow::render_start() const
    {
        bool ret{ true };
        ret &= this->swap_buffers();
        ret &= m_gui_canvas->draw_setup();
        return ret;
    }

    bool MainWindow::render_end() const
    {
        return this->swap_buffers();
    }

    [[maybe_unused]]
    bool MainWindow::swap_buffers() const
    {
        return m_gl_renderer->swap_buffers(*this);
    }

    bool MainWindow::render()
    {
        const i32 result{ SDL3::SDL_GL_MakeCurrent(m_sdl_window.get(), m_gl_renderer->gl_context()) };
        sdl_assert(result == 0, "failed to make context current");

        this->clear();
        this->gui()->redraw();  // TODO: remove!
        this->gui()->draw_all();
        this->swap_buffers();
        return result == 0;
    }

    void MainWindow::mouse_entered_event_callback(const SDL3::SDL_Event&)
    {
        ds::point<f32> delta{ 0.0f, 0.0f };
        SDL3::SDL_GetRelativeMouseState(&delta.x, &delta.y);
        m_mouse.process_motion_delta(delta);
        m_gui_canvas->on_mouse_entered(m_mouse);
    }

    void MainWindow::mouse_exited_event_callback(const SDL3::SDL_Event&)
    {
        ds::point<f32> delta{ 0.0f, 0.0f };
        SDL3::SDL_GetRelativeMouseState(&delta.x, &delta.y);
        m_mouse.process_motion_delta(delta);
        m_gui_canvas->on_mouse_exited(m_mouse);
    }

    void MainWindow::mouse_moved_event_callback(const SDL3::SDL_Event& e)
    {
        m_mouse.process_motion(e.motion);
        // update button states from pressed to held if the
        // button that was pressed last frame is still down
        if (m_mouse.is_button_pressed(Mouse::Button::Left))
            m_mouse.process_button_down(Mouse::Button::Left);

        m_gui_canvas->on_mouse_move_event(m_mouse, m_keyboard);
    }

    void MainWindow::mouse_wheel_event_callback(const SDL3::SDL_Event& e)
    {
        m_mouse.process_wheel(e.wheel);
        m_gui_canvas->on_mouse_scroll_event(m_mouse, m_keyboard);
    }

    void MainWindow::mouse_button_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        const Mouse::Button::ID button_pressed{ e.button.button };
        m_mouse.process_button_down(button_pressed);
        m_gui_canvas->on_mouse_button_pressed_event(m_mouse, m_keyboard);
    }

    void MainWindow::mouse_button_released_event_callback(const SDL3::SDL_Event& e)
    {
        const Mouse::Button::ID button_released{ e.button.button };
        m_mouse.process_button_up(button_released);
        m_gui_canvas->on_mouse_button_released_event(m_mouse, m_keyboard);
    }

    void MainWindow::keyboard_key_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        const auto pressed_button{ static_cast<Keyboard::Scancode>(e.key.keysym.scancode) };
        m_keyboard.process_button_down(pressed_button);
        m_gui_canvas->on_key_pressed(m_keyboard);
    }

    void MainWindow::keyboard_key_released_event_callback(const SDL3::SDL_Event& e)
    {
        const auto released_button{ static_cast<Keyboard::Scancode>(e.key.keysym.scancode) };
        m_keyboard.process_button_up(released_button);
        m_gui_canvas->on_key_released(m_keyboard);
    }

    void MainWindow::keyboard_char_event_callback(const SDL3::SDL_Event& e)
    {
        switch (e.type) {
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
        m_mouse.process_motion(e.motion);

        const ds::dims<i32>& window_size{ this->get_size() };
        const ds::dims<i32>& framebuf_size{ this->get_render_size() };
        const ds::dims render_size{ static_cast<ds::dims<f32>>(window_size) / m_pixel_ratio };
        debug_assert(framebuf_size.area() > 0 && window_size.area() > 0,
                     "invalid window size/location");

        m_framebuf_size = framebuf_size;
        m_window_rect.size = render_size;
        m_gl_renderer->set_viewport(ds::rect{
            ds::point<i32>{ 0, 0 },
            m_window_rect.size,
        });

        m_gui_canvas->on_resized(render_size);
        this->render();
    }

    void MainWindow::window_moved_event_callback(const SDL3::SDL_Event&)
    {
        const ds::point<i32> window_pos{ this->get_position() };
        const ds::dims<i32> window_size{ this->get_size() };
        const ds::dims<i32> framebuf_size{ this->get_render_size() };
        debug_assert(framebuf_size.area() > 0 && window_size.area() > 0,
                     "invalid window size/location");

        m_framebuf_size = framebuf_size;
        m_window_rect.pt = window_pos;
        m_window_rect.size = ds::dims<i32>{
            static_cast<i32>(static_cast<f32>(window_size.width) / m_pixel_ratio),
            static_cast<i32>(static_cast<f32>(window_size.height) / m_pixel_ratio),
        };
    }

    void MainWindow::window_pixel_size_changed_event_callback(const SDL3::SDL_Event&)
    {
        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window.get());
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);
        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window.get());
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);
    }

    void MainWindow::window_focus_gained_event_callback(const SDL3::SDL_Event&) const
    {
        m_gui_canvas->on_focus_gained();
    }

    void MainWindow::window_focus_lost_event_callback(const SDL3::SDL_Event&) const
    {
        m_gui_canvas->on_focus_lost();
    }

    void MainWindow::window_shown_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_occluded_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_hidden_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_exposed_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_minimized_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_maximized_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_restored_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_close_requested_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_take_focus_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_hit_test_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_icc_profile_changed_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_display_changed_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_display_scale_changed_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }

    void MainWindow::window_destroyed_event_callback(const SDL3::SDL_Event&) const
    {
        return;
    }
}
