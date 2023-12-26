#include <glad/gl.h>

#include <utility>

#include <nanovg.h>

#include "core/assert.hpp"
#include "core/renderer.hpp"
#include "core/ui/layout.hpp"
#include "core/window.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "sdl/utils.hpp"
#include "utils/numeric.hpp"
#include "utils/options.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {
    Window::Window(std::string title, const ds::dims<i32>& dims, Window::Properties flags)
        : ui::widget(nullptr)
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
        m_renderer = std::make_unique<rl::Renderer>(*this, rl::Renderer::DEFAULT_PROPERTY_FLAGS);
        m_nvg_context = m_renderer->nvg_context();
        sdl_assert(m_sdl_window != nullptr, "failed to create SDL_Window");
        sdl_assert(m_renderer != nullptr, "failed to create sdl::Renderer");

        this->init_gui();
    }

    bool Window::init_gui()
    {
        // Screen::Screen()
        i32 depth_bits{ 0 };
        glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);
        m_depth_buffer = depth_bits > 0;

        i32 stencil_bits{ 0 };
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil_bits);
        m_stencil_buffer = stencil_bits > 0;

        u8 float_mode{ 0 };
        glGetBooleanv(GL_RGBA_FLOAT_MODE_ARB, &float_mode);
        m_float_buffer = float_mode != 0;

        ds::dims<i32> window_size{ this->get_size() };
        ds::dims<i32> render_size{ this->get_render_size() };

        m_pixel_ratio = 1.0f;

        // Screen::initialize
        this->get_display();
        this->set_visible(true);
        this->set_theme(new ui::theme(m_nvg_context));
        this->on_mouse_move(m_display_id, ds::point{ 0, 0 });
        m_last_interaction = m_timer.elapsed();
        m_mouse_state = m_modifiers = 0;
        m_process_events = true;
        m_drag_active = false;
        m_redraw = true;

        SDL3::SDL_GL_SetSwapInterval(m_vsync);

        return true;
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
        m_window_rect = std::move(other.m_window_rect);

        return *this;
    }

    bool Window::set_opengl_attribute(OpenGL::Attribute attr, auto val)
    {
        i32 result = SDL3::SDL_GL_SetAttribute(OpenGL::sdl_type(attr), i32(val));
        sdl_assert(result == 0, "failed to set OpenGL attribute");
        return result == 0;
    }

    bool Window::clear()
    {
        return m_renderer->clear(m_background_color);
    }

    bool Window::draw_setup()
    {
        bool ret{ true };
        int result = SDL3::SDL_GL_MakeCurrent(m_sdl_window, m_renderer->gl_context());
        sdl_assert(result == 0, "failed to make context current");
        this->get_size();
        this->get_render_size();
        ret &= m_renderer->set_viewport({ 0, 0, m_fb_size.width, m_fb_size.height });
        ret &= this->swap_buffers();
        return ret;
    }

    bool Window::draw_contents()
    {
        return this->clear();
    }

    bool Window::draw_widgets()
    {
        nvgBeginFrame(m_nvg_context, m_size.width, m_size.height, m_pixel_ratio);

        this->draw(m_nvg_context);

        float elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed > 0.5f)
        {
            const ui::widget* widget{ find_widget(m_mouse_pos) };
            if (widget && !widget->tooltip().empty())
            {
                i32 tooltip_width{ 150 };
                f32 bounds[4] = { 0.0f };

                nvgFontFace(m_nvg_context, "sans");
                nvgFontSize(m_nvg_context, 15.0f);
                nvgTextAlign(m_nvg_context, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgTextLineHeight(m_nvg_context, 1.1f);

                ds::point<i32> pos{
                    widget->abs_position() +
                        ds::point<i32>(widget->width() / 2, widget->height() + 10),
                };

                nvgTextBounds(m_nvg_context, pos.x, pos.y, widget->tooltip().c_str(), nullptr,
                              bounds);

                i32 height{ static_cast<i32>((bounds[2] - bounds[0]) / 2.0f) };
                if (height > tooltip_width / 2)
                {
                    nvgTextAlign(m_nvg_context, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
                    nvgTextBoxBounds(m_nvg_context, pos.x, pos.y, tooltip_width,
                                     widget->tooltip().c_str(), nullptr, bounds);

                    height = (bounds[2] - bounds[0]) / 2;
                }

                i32 shift{ 0 };
                if (pos.x - height - 8 < 0)
                {
                    // Keep tooltips on screen
                    shift = pos.x - height - 8;
                    pos.x -= shift;
                    bounds[0] -= shift;
                    bounds[2] -= shift;
                }

                nvgGlobalAlpha(m_nvg_context, std::min(1.0f, 2.0f * (elapsed - 0.5f)) * 0.8f);

                nvgBeginPath(m_nvg_context);
                nvgFillColor(m_nvg_context, ds::color<f32>{ 0.0f, 0.0f, 0.0f, 1.0f });
                nvgRoundedRect(m_nvg_context, bounds[0] - 4 - height, bounds[1] - 4,
                               static_cast<i32>(bounds[2] - bounds[0]) + 8,
                               static_cast<i32>(bounds[3] - bounds[1]) + 8, 3);

                i32 px{ static_cast<i32>((bounds[2] + bounds[0]) / 2) - height + shift };

                nvgMoveTo(m_nvg_context, px, bounds[1] - 10);
                nvgLineTo(m_nvg_context, px + 7, bounds[1] + 1);
                nvgLineTo(m_nvg_context, px - 7, bounds[1] + 1);
                nvgFill(m_nvg_context);

                nvgFillColor(m_nvg_context, ds::color<f32>{ 1.0f, 1.0f, 1.0f, 1.0f });
                nvgFontBlur(m_nvg_context, 0.0f);
                nvgTextBox(m_nvg_context, pos.x - height, pos.y, tooltip_width,
                           widget->tooltip().c_str(), nullptr);
            }
        }

        nvgEndFrame(m_nvg_context);

        return true;
    }

    bool Window::redraw()
    {
        m_redraw |= true;
        // if (!m_redraw)
        // {
        //     m_redraw = true;
        //
        //     // Posts an empty event to the event queue.
        //     //
        //     // This function posts an empty event from the current thread to the event
        //     // queue, causing @ref glfwWaitEvents or @ref glfwWaitEventsTimeout to return.
        //     //
        //     // Possible errors include GLFW_NOT_INITIALIZED and GLFW_PLATFORM_ERROR.
        //     //
        //     // This function may be called from any thread.
        //     //
        //     // Related:
        //     // - events
        //     // - glfwWaitEvents
        //     // - glfwWaitEventsTimeout
        //     //
        //     // glfwPostEmptyEvent();
        // }

        return true;
    }

    bool Window::draw_teardown()
    {
        return this->swap_buffers();
    }

    bool Window::draw_all()
    {
        bool ret = true;
        ret &= this->draw_setup();
        ret &= this->draw_contents();
        ret &= this->draw_widgets();
        ret &= this->draw_teardown();
        return ret;
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

    bool Window::swap_buffers()
    {
        return m_renderer->swap_buffers(*this);
    }

    SDL3::SDL_Window* Window::sdl_handle() const
    {
        return m_sdl_window;
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

    bool Window::on_shown(const WindowID id)
    {
        this->set_visible(true);
        if constexpr (io::logging::window_events)
            log::info("window::on_shown [id:{}]", id);

        return true;
    }

    bool Window::on_hidden(const WindowID id)
    {
        this->set_visible(false);
        if constexpr (io::logging::window_events)
            log::info("window::on_hidden [id:{}]", id);

        return true;
    }

    bool Window::on_exposed(const WindowID id)
    {
        m_visible = true;
        if constexpr (io::logging::window_events)
            log::info("window::on_exposed [id:{}]", id);

        return true;
    }

    bool Window::on_moved(rl::WindowID id, ds::point<i32>&& pt)
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

        m_last_interaction = m_timer.elapsed();
        m_window_rect.size = size;

        if (m_resize_callback != nullptr)
            m_resize_callback(size);

        m_redraw = true;
        return this->draw_all();
    }

    bool Window::on_pixel_size_changed(const WindowID id, ds::dims<i32>&& pixel_size)
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

    bool Window::on_minimized(const WindowID id)
    {
        this->set_visible(false);
        if constexpr (io::logging::window_events)
            log::info("window::on_minimized [id:{}]", id);
        return true;
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
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_enter [id:{}]", id);

        return ui::widget::on_mouse_enter(id);
    }

    bool Window::on_mouse_leave(const WindowID id)
    {
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_leave [id:{}]", id);

        return ui::widget::on_mouse_leave(id);
    }

    bool Window::on_mouse_click(const Mouse::Button::type button)
    {
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_click [button:{}]", button);

        return true;
    }

    bool Window::on_mouse_drag(const WindowID id)
    {
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_drag [id:{}]", id);

        return ui::widget::on_mouse_drag(id);
    }

    bool Window::on_mouse_move(const WindowID id, ds::point<i32>&& pos)
    {
        m_mouse_pos = pos;
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_move [id:{}, pos:{}]", id, m_mouse_pos);
        return true;
    }

    bool Window::on_mouse_scroll(Mouse::Event::Data::Wheel& wheel)
    {
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_scroll [vert delta:{}]", wheel.y);

        return true;
    }

    bool Window::on_kb_focus_gained(const WindowID id)
    {
        this->set_focused(true);

        if constexpr (io::logging::window_events)
            log::info("window::on_kb_focus_gained [id:{}]", id);

        return true;
    }

    bool Window::on_kb_focus_lost(const WindowID id)
    {
        this->set_focused(false);

        if constexpr (io::logging::window_events)
            log::info("window::on_kb_focus_lost [id:{}]", id);

        return true;
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
        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);

        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);

        if constexpr (io::logging::window_events)
            log::info("window::on_display_scale_changed [id:{}, ratio:{}, density:{}]", id,
                      m_pixel_ratio, m_pixel_density);

        return m_pixel_ratio != 0.0f && m_pixel_density != 0.0f;
    }

    bool Window::on_display_content_scale_changed(const DisplayID id)
    {
        m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
        sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);

        m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
        sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]", m_window_id);

        if constexpr (io::logging::window_events)
            log::info("window::on_display_content_scale_changed [id:{}, ratio:{}, density:{}]", id,
                      m_pixel_ratio, m_pixel_density);

        return m_pixel_ratio != 0.0f && m_pixel_density != 0.0f;
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
