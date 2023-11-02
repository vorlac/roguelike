#include <functional>

#include "core/ds/color.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/conversions.hpp"
#include "core/window.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    Window::Window()
    {
        this->setup();
    }

    Window::Window(ds::dimensions<i32> dimensions, std::string title)
    {
        this->setup(dimensions.width, dimensions.height, title);
    }

    Window::~Window()
    {
        this->teardown();
    }

    void Window::begin_drawing() const
    {
        raylib::BeginDrawing();
        raylib::ClearBackground(color::darkgray);
    }

    void Window::end_drawing(bool draw_fps /* = true*/) const
    {
        if (draw_fps) [[likely]]
        {
            raylib::DrawRectangle(0, 0, 95, 40, color::black);
            raylib::DrawFPS(10, 10);
        }

        raylib::EndDrawing();
    }

    void Window::update_gui()
    {
        m_gui.update(*this, m_display);
    }

    bool Window::is_ready() const
    {
        return raylib::IsWindowReady();
    }

    bool Window::should_close() const
    {
        return raylib::WindowShouldClose();
    }

    void Window::close() const
    {
        return raylib::CloseWindow();
    }

    bool Window::is_fullscreen() const
    {
        return raylib::IsWindowFullscreen();
    }

    bool Window::is_hidden() const
    {
        return raylib::IsWindowHidden();
    }

    bool Window::is_minimized() const
    {
        return raylib::IsWindowMinimized();
    }

    bool Window::is_maximized() const
    {
        return raylib::IsWindowMaximized();
    }

    bool Window::is_focused() const
    {
        return raylib::IsWindowFocused();
    }

    bool Window::is_resized() const
    {
        return raylib::IsWindowResized();
    }

    bool Window::get_state(u32 flag) const
    {
        return raylib::IsWindowState(flag);
    }

    void Window::set_state(u32 flags) const
    {
        return raylib::SetWindowState(flags);
    }

    void Window::clear_state(u32 flags) const
    {
        return raylib::ClearWindowState(flags);
    }

    void Window::toggle_fullscreen() const
    {
        return raylib::ToggleFullscreen();
    }

    void Window::maximize() const
    {
        return raylib::MaximizeWindow();
    }

    void Window::minimize() const
    {
        return raylib::MinimizeWindow();
    }

    void Window::restore() const
    {
        return raylib::RestoreWindow();
    }

    void Window::set_icon(raylib::Image&& image) const
    {
        return raylib::SetWindowIcon(image);
    }

    void Window::set_icons(std::vector<raylib::Image>&& images) const
    {
        return raylib::SetWindowIcons(images.data(), cast::to<i32>(images.size()));
    }

    void Window::title(std::string title) const
    {
        return raylib::SetWindowTitle(title.c_str());
    }

    void Window::set_position(ds::point<i32> pos) const
    {
        return raylib::SetWindowPosition(pos.x, pos.y);
    }

    void Window::set_monitor(i16 monitor) const
    {
        return raylib::SetWindowMonitor(monitor);
    }

    void Window::min_size(ds::dimensions<i32> min_size) const
    {
        return raylib::SetWindowMinSize(min_size.width, min_size.height);
    }

    void Window::size(ds::dimensions<i32> size) const
    {
        return raylib::SetWindowSize(size.width, size.height);
    }

    void Window::opacity(f32 opacity) const
    {
        return raylib::SetWindowOpacity(opacity);
    }

    void* Window::handle() const
    {
        return raylib::GetWindowHandle();
    }

    f32 Window::frame_time() const
    {
        return raylib::GetFrameTime();
    }

    ds::point<f32> Window::center() const
    {
        return ds::point<f32>(cast::to<f32>(raylib::GetScreenWidth()) / 2.0f,
                              cast::to<f32>(raylib::GetScreenHeight()) / 2.0f);
    }

    ds::dimensions<i32> Window::screen_size() const
    {
        return {
            raylib::GetScreenWidth(),
            raylib::GetScreenHeight(),
        };
    }

    ds::dimensions<i32> Window::render_size() const
    {
        return {
            raylib::GetRenderWidth(),
            raylib::GetRenderHeight(),
        };
    }

    ds::point<f32> Window::position() const
    {
        auto pos{ raylib::GetWindowPosition() };
        return {
            pos.x,
            pos.y,
        };
    }

    ds::vector2<f32> Window::scale_dpi_factor() const
    {
        const auto dpi{ raylib::GetWindowScaleDPI() };
        return ds::vector2<f32>(dpi.x, dpi.y);
    }

    bool Window::setup(i32 width, i32 height, std::string title)
    {
        constexpr i32 flags{ raylib::ConfigFlags::FLAG_MSAA_4X_HINT |
                             raylib::ConfigFlags::FLAG_WINDOW_RESIZABLE |
                             raylib::ConfigFlags::FLAG_VSYNC_HINT };

        raylib::SetConfigFlags(flags);
        raylib::InitWindow(width, height, title.c_str());
        m_gui.setup(*this);
        return true;
    }

    bool Window::teardown()
    {
        m_gui.teardown();
        if (this->is_ready())
            this->close();

        return true;
    }
}
