#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <raylib.h>

#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"

namespace rl
{
    class Window
    {
    public:
        Window()
        {
            this->init();
        }

        Window(ds::dimensions<int32_t> dimensions, const std::string& title = "")
        {
            this->init(dimensions.width, dimensions.height, title);
        }

        ~Window()
        {
            this->teardown();
        }

        void render(auto render_func) const
        {
            this->begin_drawing();
            render_func();
            this->end_drawing();
        }

        void begin_drawing() const
        {
            return ::BeginDrawing();
        }

        void end_drawing() const
        {
            return ::EndDrawing();
        }

        bool is_ready() const
        {
            return ::IsWindowReady();
        }

        bool should_close() const
        {
            return ::WindowShouldClose();
        }

        void close() const
        {
            return ::CloseWindow();
        }

        inline bool is_fullscreen() const
        {
            return ::IsWindowFullscreen();
        }

        bool is_hidden() const
        {
            return ::IsWindowHidden();
        }

        bool is_minimized() const
        {
            return ::IsWindowMinimized();
        }

        bool is_maximized() const
        {
            return ::IsWindowMaximized();
        }

        bool is_focused() const
        {
            return ::IsWindowFocused();
        }

        bool is_resized() const
        {
            return ::IsWindowResized();
        }

        bool is_state(uint32_t flag) const
        {
            return ::IsWindowState(flag);
        }

        void set_state(uint32_t flags) const
        {
            return ::SetWindowState(flags);
        }

        void clear_state(uint32_t flags) const
        {
            return ::ClearWindowState(flags);
        }

        void toggle_fullscreen() const
        {
            return ::ToggleFullscreen();
        }

        void maximize() const
        {
            return ::MaximizeWindow();
        }

        void minimize() const
        {
            return ::MinimizeWindow();
        }

        void restore() const
        {
            return ::RestoreWindow();
        }

        void set_icon(::Image&& image) const
        {
            return ::SetWindowIcon(image);
        }

        void set_icons(std::vector<::Image>&& images) const
        {
            return ::SetWindowIcons(images.data(), static_cast<int>(images.size()));
        }

        void title(std::string&& title) const
        {
            return ::SetWindowTitle(title.c_str());
        }

        void set_position(ds::position<int32_t>&& pos) const
        {
            return ::SetWindowPosition(pos.x, pos.y);
        }

        void set_monitor(uint16_t monitor) const
        {
            return ::SetWindowMonitor(monitor);
        }

        void min_size(ds::dimensions<int32_t> min_size) const
        {
            return ::SetWindowMinSize(min_size.width, min_size.height);
        }

        void size(ds::dimensions<int32_t> size) const
        {
            return ::SetWindowSize(size.width, size.height);
        }

        void opacity(float opacity) const
        {
            return ::SetWindowOpacity(opacity);
        }

        void* handle() const
        {
            return ::GetWindowHandle();
        }

        ds::dimensions<int32_t> screen_size() const
        {
            return {
                .width = ::GetScreenWidth(),
                .height = ::GetScreenHeight(),
            };
        }

        ds::dimensions<int32_t> render_size() const
        {
            return {
                .width = ::GetRenderWidth(),
                .height = ::GetRenderHeight(),
            };
        }

        ds::point<float> position() const
        {
            return ::GetWindowPosition();
        }

        ds::vector2<float> scale_dpi_factor() const
        {
            return ::GetWindowScaleDPI();
        }

    public:
        Window& operator=(Window window) = delete;
        Window& operator=(Window& window) = delete;
        Window& operator=(Window&& window) = delete;
        Window& operator=(const Window& window) = delete;

    protected:
        bool init(int32_t width = 1024, int32_t height = 768, const std::string& title = "") const
        {
            ::InitWindow(width, height, title.c_str());
            return true;
        }

        bool teardown() const
        {
            if (this->is_ready())
                this->close();

            return true;
        }
    };
}
