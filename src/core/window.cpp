#include <functional>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/window.hpp"

#include "thirdparty/raylib.hpp"

namespace rl
{
    Window::Window()
    {
        this->setup();
    }

    Window::Window(ds::dimensions<int32_t> dimensions, std::string title)
    {
        this->setup(dimensions.width, dimensions.height, title);
    }

    Window::~Window()
    {
        this->teardown();
    }

    void Window::begin_drawing()
    {
        return raylib::BeginDrawing();
    }

    void Window::end_drawing()
    {
        return raylib::EndDrawing();
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

    // void Window::set_icon(raylib::Image&& image)
    //{
    //     return raylib::SetWindowIcon(image);
    // }

    // void Window::set_icons(std::vector<raylib::Image> images)
    //{
    //     return raylib::SetWindowIcons(images.data(), static_cast<int>(images.size()));
    // }

    void Window::title(std::string title)
    {
        return raylib::SetWindowTitle(title.c_str());
    }

    void Window::set_position(ds::position<int32_t> pos)
    {
        return raylib::SetWindowPosition(pos.x, pos.y);
    }

    void Window::set_monitor(uint16_t monitor)
    {
        return raylib::SetWindowMonitor(monitor);
    }

    void Window::min_size(ds::dimensions<int32_t> min_size)
    {
        return raylib::SetWindowMinSize(min_size.width, min_size.height);
    }

    void Window::size(ds::dimensions<int32_t> size)
    {
        return raylib::SetWindowSize(size.width, size.height);
    }

    void Window::opacity(float opacity)
    {
        return raylib::SetWindowOpacity(opacity);
    }

    void* Window::handle()
    {
        return raylib::GetWindowHandle();
    }

    ds::position<float> Window::center()
    {
        return ds::point<float>(static_cast<float>(raylib::GetScreenWidth()) / 2.0f,
                                static_cast<float>(raylib::GetScreenHeight()) / 2.0f);
    }

    ds::dimensions<int32_t> Window::screen_size()
    {
        return {
            raylib::GetScreenWidth(),
            raylib::GetScreenHeight(),
        };
    }

    ds::dimensions<int32_t> Window::render_size()
    {
        return {
            raylib::GetRenderWidth(),
            raylib::GetRenderHeight(),
        };
    }

    ds::position<float> Window::position()
    {
        auto pos{ raylib::GetWindowPosition() };
        return {
            pos.x,
            pos.y,
        };
    }

    ds::vector2<float> Window::scale_dpi_factor()
    {
        auto dpi{ raylib::GetWindowScaleDPI() };
        return ds::vector2<float>(dpi.x, dpi.y);
    }

    bool Window::setup(int32_t width, int32_t height, std::string title)
    {
        raylib::SetConfigFlags(raylib::ConfigFlags::FLAG_MSAA_4X_HINT |
                               raylib::ConfigFlags::FLAG_WINDOW_RESIZABLE);
        raylib::InitWindow(width, height, title.c_str());
        return true;
    }

    bool Window::teardown()
    {
        if (this->is_ready())
            this->close();

        return true;
    }
}
