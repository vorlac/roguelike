#include "core/window.hpp"

#include <functional>
#include <raylib.h>

#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"

namespace rl
{
    Window::Window()
    {
        this->init();
    }

    Window::Window(ds::dimensions<int32_t> dimensions, std::string title)
    {
        this->init(dimensions.width, dimensions.height, title);
    }

    Window::~Window()
    {
        this->teardown();
    }

    void Window::begin_drawing()
    {
        return ::BeginDrawing();
    }

    void Window::end_drawing()
    {
        return ::EndDrawing();
    }

    bool Window::is_ready()
    {
        return ::IsWindowReady();
    }

    bool Window::should_close() const
    {
        return ::WindowShouldClose();
    }

    void Window::close() const
    {
        return ::CloseWindow();
    }

    bool Window::is_fullscreen()
    {
        return ::IsWindowFullscreen();
    }

    bool Window::is_hidden()
    {
        return ::IsWindowHidden();
    }

    bool Window::is_minimized()
    {
        return ::IsWindowMinimized();
    }

    bool Window::is_maximized()
    {
        return ::IsWindowMaximized();
    }

    bool Window::is_focused()
    {
        return ::IsWindowFocused();
    }

    bool Window::is_resized()
    {
        return ::IsWindowResized();
    }

    bool Window::get_state(uint32_t flag)
    {
        return ::IsWindowState(flag);
    }

    void Window::set_state(uint32_t flags)
    {
        return ::SetWindowState(flags);
    }

    void Window::clear_state(uint32_t flags)
    {
        return ::ClearWindowState(flags);
    }

    void Window::toggle_fullscreen()
    {
        return ::ToggleFullscreen();
    }

    void Window::maximize()
    {
        return ::MaximizeWindow();
    }

    void Window::minimize()
    {
        return ::MinimizeWindow();
    }

    void Window::restore()
    {
        return ::RestoreWindow();
    }

    void Window::set_icon(::Image&& image)
    {
        return ::SetWindowIcon(image);
    }

    void Window::set_icons(std::vector<::Image> images)
    {
        return ::SetWindowIcons(images.data(), static_cast<int>(images.size()));
    }

    void Window::title(std::string title)
    {
        return ::SetWindowTitle(title.c_str());
    }

    void Window::set_position(ds::position<int32_t> pos)
    {
        return ::SetWindowPosition(pos.x, pos.y);
    }

    void Window::set_monitor(uint16_t monitor)
    {
        return ::SetWindowMonitor(monitor);
    }

    void Window::min_size(ds::dimensions<int32_t> min_size)
    {
        return ::SetWindowMinSize(min_size.width, min_size.height);
    }

    void Window::size(ds::dimensions<int32_t> size)
    {
        return ::SetWindowSize(size.width, size.height);
    }

    void Window::opacity(float opacity)
    {
        return ::SetWindowOpacity(opacity);
    }

    void* Window::handle()
    {
        return ::GetWindowHandle();
    }

    ds::point<float> Window::center()
    {
        return ds::point<float>(static_cast<float>(::GetScreenWidth()) / 2.0f,
                                static_cast<float>(::GetScreenHeight()) / 2.0f);
    }

    ds::dimensions<int32_t> Window::screen_size()
    {
        return {
            .width = ::GetScreenWidth(),
            .height = ::GetScreenHeight(),
        };
    }

    ds::dimensions<int32_t> Window::render_size()
    {
        return {
            .width = ::GetRenderWidth(),
            .height = ::GetRenderHeight(),
        };
    }

    ds::point<float> Window::position()
    {
        auto pos{ ::GetWindowPosition() };
        return ds::point<float>(pos.x, pos.y);
    }

    ds::vector2<float> Window::scale_dpi_factor()
    {
        auto dpi{ ::GetWindowScaleDPI() };
        return ds::vector2<float>(dpi.x, dpi.y);
    }

    bool Window::init(int32_t width, int32_t height, std::string title)
    {
        ::SetConfigFlags(::ConfigFlags::FLAG_MSAA_4X_HINT);
        ::InitWindow(width, height, title.c_str());
        return true;
    }

    bool Window::teardown()
    {
        if (this->is_ready())
            this->close();

        return true;
    }
}
