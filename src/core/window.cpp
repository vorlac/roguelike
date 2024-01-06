#include <glad/gl.h>

#include <array>
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
    Window::Window(ui::widget* parent, const std::string& title)
        : ui::widget{ parent }
        , m_title{ title }
        , m_modal{ false }
        , m_screen{ new ui::Screen(m_nvg_context) }
        , m_button_panel{ nullptr }
    {
    }

    Window::Window(std::string title, const ds::dims<i32>& dims, Window::Properties flags)
        : ui::widget{ nullptr }
        , m_button_panel{ nullptr }
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
        m_window_rect = { m_sdl_window ? this->get_position() : ds::point<i32>::null(), dims };
        m_renderer = std::make_unique<Renderer>(*this, Renderer::DEFAULT_PROPERTY_FLAGS);
        m_nvg_context = m_renderer->nvg_context();

        sdl_assert(m_sdl_window != nullptr, "failed to create SDL_Window");
        sdl_assert(m_renderer != nullptr, "failed to create sdl::Renderer");

        this->get_display();
        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> render_size{ this->get_render_size() };

        SDL3::SDL_GL_SetSwapInterval(m_vsync);

        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            m_cursors[i] = SDL3::SDL_CreateSystemCursor(Mouse::Cursor::type(i));

        m_screen = new ui::Screen(m_nvg_context);
    }

    Window::~Window()
    {
        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            if (m_cursors[i] != nullptr)
                SDL3::SDL_DestroyCursor(m_cursors[i]);

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
        ui::widget::set_size(size);
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

    const ds::color<u8>& Window::background() const
    {
        return m_background_color;
    }

    bool Window::set_background(ds::color<u8> background)
    {
        m_background_color = background;
        return true;
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
        i32 result = SDL3::SDL_GetWindowSize(m_sdl_window, &m_size.width, &m_size.height);
        sdl_assert(result == 0, "failed to set size");
        return m_size;
    }

    ds::dims<i32> Window::get_render_size()
    {
        i32 result = SDL3::SDL_GetWindowSizeInPixels(m_sdl_window, &m_fb_size.width,
                                                     &m_fb_size.height);
        sdl_assert(result == 0, "failed to set render size");

        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);
        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);

        return m_fb_size;
    }

    bool Window::set_opengl_attribute(OpenGL::Attribute attr, auto val)
    {
        i32 result = SDL3::SDL_GL_SetAttribute(OpenGL::sdl_type(attr), i32(val));
        sdl_assert(result == 0, "failed to set OpenGL attribute");
        return result == 0;
    }

    bool Window::on_pixel_size_changed(const WindowID id, ds::dims<i32> pixel_size)
    {
        bool ret = true;

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
        return m_renderer->clear(m_background_color);
    }

    bool Window::render_start()
    {
        i32 result{ SDL3::SDL_GL_MakeCurrent(m_sdl_window, m_renderer->gl_context()) };
        sdl_assert(result == 0, "failed to make context current");

        this->get_size();
        this->get_render_size();

        bool ret{ result == 0 };

        ret &= m_renderer->set_viewport({
            0,
            0,
            m_fb_size.width,
            m_fb_size.height,
        });

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

    void Window::render()
    {
        m_screen->draw(m_nvg_context);

        // TODO: remove / move
        m_renderer->clear(m_background_color);
    }

    ds::dims<i32> Window::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_button_panel != nullptr)
            m_button_panel->hide();

        ds::dims<i32> result{ ui::widget::preferred_size(nvg_context) };

        if (m_button_panel != nullptr)
            m_button_panel->show();

        nvgFontSize(nvg_context, 18.0f);
        nvgFontFace(nvg_context, ui::font::name::sans_bold);

        std::array<f32, 4> bounds = {};
        nvgTextBounds(nvg_context, 0, 0, m_title.c_str(), nullptr, bounds.data());

        return ds::dims<i32>(std::max(result.width, static_cast<i32>(bounds[2] - bounds[0] + 20)),
                             std::max(result.height, static_cast<i32>(bounds[3] - bounds[1])));
    }

    void Window::refresh_relative_placement()
    {
        // Overridden in ui::Popup
        return;
    }

    void Window::mouse_entered_event_callback(const SDL3::SDL_Event& e)
    {
        m_screen->on_mouse_entered(m_mouse);
    }

    void Window::mouse_exited_event_callback(const SDL3::SDL_Event& e)
    {
        m_screen->on_mouse_exited(m_mouse);
    }

    const std::string& Window::title() const
    {
        return m_title;
    }

    bool Window::modal() const
    {
        return m_modal;
    }

    bool Window::set_modal(bool modal)
    {
        m_modal = modal;
        return m_modal;
    }

    ui::widget* Window::button_panel()
    {
        if (!m_button_panel)
        {
            m_button_panel = new ui::widget{ this };
            m_button_panel->set_layout({
                new ui::box_layout{
                    ui::orientation::Horizontal,
                    ui::alignment::Middle,
                    0,
                    4,
                },
            });
        }

        return m_button_panel;
    }

    void Window::dispose()
    {
        ui::widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        ((ui::Screen*)owner)->dispose_window(this);
    }

    void Window::center()
    {
        ui::widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        ((ui::Screen*)owner)->center_window(this);
    }

    // void cursor_pos_callback_event(double x, double y);
    void Window::mouse_moved_event_callback(const SDL3::SDL_Event& e)
    {
        if constexpr (io::logging::mouse_events)
            log::info("{}", m_mouse);

        m_mouse.process_motion(e.motion);
        ds::point<i32> mouse_pos{ m_mouse.pos() };

        bool ret{ false };

        ds::point<i32> pnt{
            static_cast<i32>(std::round(mouse_pos.x / m_pixel_ratio)),
            static_cast<i32>(std::round(mouse_pos.y / m_pixel_ratio)),
        };

        // TODO: figure out what thsi does...
        pnt -= ds::vector2<i32>{ 1, 2 };

        m_screen->m_last_interaction = m_timer.elapsed();
        if (!m_screen->m_drag_active)
        {
            ui::widget* widget{ this->find_widget(pnt) };
            if (widget != nullptr && widget->cursor() != m_cursor)
            {
                m_cursor = widget->cursor();
                SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
                runtime_assert(widget_cursor != nullptr, "invalid cursor");
                SDL3::SDL_SetCursor(widget_cursor);
            }
        }
        else
        {
            auto&& pos{ m_mouse.pos() };
            ret = m_screen->m_drag_widget->on_mouse_drag(
                pnt - m_screen->m_drag_widget->parent()->abs_position(), pnt - mouse_pos, m_mouse,
                m_keyboard);
        }

        ret = ret || this->on_mouse_move(m_mouse, m_keyboard);
        m_screen->m_redraw |= ret;
    }

    void Window::mouse_wheel_event_callback(const SDL3::SDL_Event& e)
    {
        if constexpr (io::logging::mouse_events)
            log::info("{}", m_mouse);

        m_screen->m_last_interaction = m_timer.elapsed();
        m_mouse.process_wheel(e.wheel);

        ds::vector2<i32> wheel_pos{ m_mouse.wheel() };
        ds::point<i32> mouse_pos{ m_mouse.pos() };

        if (m_screen->m_focus_path.size() > 1)
        {
            const Window* window = dynamic_cast<Window*>(
                m_screen->m_focus_path[m_screen->m_focus_path.size() - 2]);
            if (window && window->modal())
            {
                if (!window->contains(m_mouse.pos()))
                    return;
            }
        }

        m_screen->m_redraw |= this->on_mouse_scroll(m_mouse, m_keyboard);
    }

    void Window::mouse_button_released_event_callback(const SDL3::SDL_Event& e)
    {
        m_screen->m_last_interaction = m_timer.elapsed();
        ds::point<i32> mouse_pos{ m_mouse.pos() };

        if (m_screen->m_focus_path.size() > 1)
        {
            const Window* window{ dynamic_cast<Window*>(
                m_screen->m_focus_path[m_screen->m_focus_path.size() - 2]) };
            if (window != nullptr && window->modal())
            {
                if (!window->contains(mouse_pos))
                    return;
            }
        }

        Mouse::Button::type released_button{ e.button.button };

        // unset button state
        m_mouse.process_button_up(released_button);
        if constexpr (io::logging::mouse_events)
            log::info("{}", m_mouse);

        auto drop_widget{ this->find_widget(mouse_pos) };
        if (m_screen->m_drag_active && drop_widget != m_screen->m_drag_widget)
            m_screen->m_redraw |= m_screen->m_drag_widget->on_mouse_button_released(m_mouse,
                                                                                    m_keyboard);

        if (m_screen->m_drag_active && drop_widget != nullptr && m_cursor != drop_widget->cursor())
        {
            m_cursor = drop_widget->cursor();
            SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
            runtime_assert(widget_cursor != nullptr, "invalid cursor");
            SDL3::SDL_SetCursor(widget_cursor);
        }

        const bool drag_btn_released{ m_mouse.is_button_released(Mouse::Button::Left) };
        if (m_screen->m_drag_active && drag_btn_released)
        {
            m_screen->m_drag_active = false;
            m_screen->m_drag_widget = nullptr;
        }

        m_screen->m_redraw |= this->on_mouse_button_released(m_mouse, m_keyboard);
    }

    void Window::mouse_button_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        m_screen->m_last_interaction = m_timer.elapsed();
        ds::point<i32> mouse_pos{ m_mouse.pos() };

        if (m_screen->m_focus_path.size() > 1)
        {
            const Window* window{ dynamic_cast<Window*>(
                m_screen->m_focus_path[m_screen->m_focus_path.size() - 2]) };
            if (window != nullptr && window->modal())
            {
                if (!window->contains(mouse_pos))
                    return;
            }
        }

        // set button state
        const auto button_pressed{ e.button.button };
        m_mouse.process_button_down(button_pressed);
        if constexpr (io::logging::mouse_events)
            log::info("{}", m_mouse);

        auto drop_widget{ this->find_widget(mouse_pos) };
        if (drop_widget != nullptr && m_cursor != drop_widget->cursor())
        {
            m_cursor = drop_widget->cursor();
            SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
            runtime_assert(widget_cursor != nullptr, "invalid cursor");
            SDL3::SDL_SetCursor(widget_cursor);
        }

        const bool drag_btn_pressed{ m_mouse.is_button_pressed(Mouse::Button::Left) };
        if (!m_screen->m_drag_active && drag_btn_pressed)
        {
            m_screen->m_drag_widget = this->find_widget(mouse_pos);
            if (m_screen->m_drag_widget == this)
                m_screen->m_drag_widget = nullptr;

            m_screen->m_drag_active = m_screen->m_drag_widget != nullptr;
            if (!m_screen->m_drag_active)
                m_screen->update_focus(nullptr);
        }

        m_screen->m_redraw |= this->on_mouse_button_pressed(m_mouse, m_keyboard);
    }

    void Window::keyboard_key_pressed_event_callback(const SDL3::SDL_Event& e)
    {
        if constexpr (io::logging::kb_events)
            log::info("{}", m_keyboard);

        m_screen->m_last_interaction = m_timer.elapsed();
        Keyboard::Button::type pressed_button(e.key.keysym.scancode);
        m_keyboard.process_button_down(pressed_button);
        m_screen->m_redraw |= this->on_key_pressed(m_keyboard);
    }

    void Window::keyboard_key_released_event_callback(const SDL3::SDL_Event& e)
    {
        if constexpr (io::logging::kb_events)
            log::info("{}", m_keyboard);

        m_screen->m_last_interaction = m_timer.elapsed();
        Keyboard::Button::type released_button(e.key.keysym.scancode);
        m_keyboard.process_button_up(released_button);
        m_screen->m_redraw |= this->on_key_released(m_keyboard);
    }

    void Window::keyboard_char_event_callback(const SDL3::SDL_Event& e)
    {
        if constexpr (io::logging::kb_events)
            log::info("{}", m_keyboard);

        m_screen->m_last_interaction = m_timer.elapsed();
        switch (e.type)
        {
            case Keyboard::Event::TextInput:
                m_keyboard.process_text_input(e.text.text);
                break;
            case Keyboard::Event::TextEditing:
                m_keyboard.process_text_editing(e.edit.text, e.edit.start, e.edit.length);
                break;
        }

        m_screen->m_redraw |= this->on_character_input(m_keyboard);
    }

    // void resize_callback_event(int width, int height);
    void Window::window_resized_event_callback(const SDL3::SDL_Event& e)
    {
        // TODO: refactor size / render_size. i think two variants will be necessary. one that calls
        // SDL, another than just gets cached variables (m_size and m_renderer->framebuf_size())
        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> framebuf_size{ this->get_render_size() };
        if (framebuf_size.area() == 0 || window_size.area() == 0)
            return;

        m_framebuf_size = framebuf_size;
        m_window_rect.size = ds::dims<i32>{
            static_cast<i32>(this->width() / m_pixel_ratio),
            static_cast<i32>(this->height() / m_pixel_ratio),
        };

        // TODO: seperate window / GUI viewport
        // so this can be removed
        //
        // this is widget::m_size NOT window::m_size
        m_size = m_window_rect.size;
        m_screen->m_last_interaction = m_timer.elapsed();

        // m_screen->on_resized(m_size);
        m_screen->redraw();
    }
}
