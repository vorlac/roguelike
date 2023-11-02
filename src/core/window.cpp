#include <functional>

#include "core/ds/color.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/window.hpp"
#include "thirdparty/raylib.hpp"
#include "thirdparty/rlimgui.hpp"

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

    void Window::begin_drawing()
    {
        raylib::BeginDrawing();
        rlimgui::Begin();
        raylib::ClearBackground(color::darkgray);
    }

    void Window::end_drawing(bool draw_fps /* = true*/)
    {
        if (draw_fps) [[likely]]
        {
            raylib::DrawRectangle(0, 0, 95, 40, color::black);
            raylib::DrawFPS(10, 10);
        }

        rlimgui::End();
        raylib::EndDrawing();
    }

    bool Window::is_ready()
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

    bool Window::is_fullscreen()
    {
        return raylib::IsWindowFullscreen();
    }

    bool Window::is_hidden()
    {
        return raylib::IsWindowHidden();
    }

    bool Window::is_minimized()
    {
        return raylib::IsWindowMinimized();
    }

    bool Window::is_maximized()
    {
        return raylib::IsWindowMaximized();
    }

    bool Window::is_focused()
    {
        return raylib::IsWindowFocused();
    }

    bool Window::is_resized()
    {
        return raylib::IsWindowResized();
    }

    bool Window::get_state(u32 flag)
    {
        return raylib::IsWindowState(flag);
    }

    void Window::set_state(u32 flags)
    {
        return raylib::SetWindowState(flags);
    }

    void Window::clear_state(u32 flags)
    {
        return raylib::ClearWindowState(flags);
    }

    void Window::toggle_fullscreen()
    {
        return raylib::ToggleFullscreen();
    }

    void Window::maximize()
    {
        return raylib::MaximizeWindow();
    }

    void Window::minimize()
    {
        return raylib::MinimizeWindow();
    }

    void Window::restore()
    {
        return raylib::RestoreWindow();
    }

    void Window::set_icon(raylib::Image&& image)
    {
        return raylib::SetWindowIcon(image);
    }

    void Window::set_icons(std::vector<raylib::Image>&& images)
    {
        return raylib::SetWindowIcons(images.data(), static_cast<int>(images.size()));
    }

    void Window::title(std::string title)
    {
        return raylib::SetWindowTitle(title.c_str());
    }

    void Window::set_position(ds::point<i32> pos)
    {
        return raylib::SetWindowPosition(pos.x, pos.y);
    }

    void Window::set_monitor(i16 monitor)
    {
        return raylib::SetWindowMonitor(monitor);
    }

    void Window::min_size(ds::dimensions<i32> min_size)
    {
        return raylib::SetWindowMinSize(min_size.width, min_size.height);
    }

    void Window::size(ds::dimensions<i32> size)
    {
        return raylib::SetWindowSize(size.width, size.height);
    }

    void Window::opacity(f32 opacity)
    {
        return raylib::SetWindowOpacity(opacity);
    }

    void* Window::handle()
    {
        return raylib::GetWindowHandle();
    }

    ds::point<f32> Window::center()
    {
        return ds::point<f32>(static_cast<f32>(raylib::GetScreenWidth()) / 2.0f,
                              static_cast<f32>(raylib::GetScreenHeight()) / 2.0f);
    }

    ds::dimensions<i32> Window::screen_size()
    {
        return {
            raylib::GetScreenWidth(),
            raylib::GetScreenHeight(),
        };
    }

    ds::dimensions<i32> Window::render_size()
    {
        return {
            raylib::GetRenderWidth(),
            raylib::GetRenderHeight(),
        };
    }

    ds::point<f32> Window::position()
    {
        auto pos{ raylib::GetWindowPosition() };
        return {
            pos.x,
            pos.y,
        };
    }

    ds::vector2<f32> Window::scale_dpi_factor()
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
        rlimgui::Setup(true);
        return true;
    }

    bool Window::teardown()
    {
        rlimgui::Shutdown();
        if (this->is_ready())
            this->close();

        return true;
    }
}
