#pragma once

#include <string>
#include <vector>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"

namespace rl
{
    class Window
    {
    public:
        Window();
        Window(ds::dimensions<int32_t> dimensions, std::string title);

        ~Window();

        void render(auto render_func)
        {
            this->begin_drawing();
            render_func();
            this->end_drawing();
        }

        void begin_drawing();
        void end_drawing();

        bool should_close() const;
        void close() const;

        bool is_ready();
        bool is_hidden();
        bool is_fullscreen();
        bool is_minimized();
        bool is_maximized();
        bool is_focused();
        bool is_resized();

        bool get_state(uint32_t flag);
        void set_state(uint32_t flags);
        void clear_state(uint32_t flags);

        void toggle_fullscreen();
        void maximize();
        void minimize();
        void restore();

        void set_icon(raylib::Image&& image);
        void set_icons(std::vector<raylib::Image> images);

        void title(std::string title);

        void set_position(ds::position<int32_t> pos);
        void set_monitor(uint16_t monitor);
        void min_size(ds::dimensions<int32_t> min_size);
        void size(ds::dimensions<int32_t> size);
        void opacity(float opacity);

        void* handle();

        ds::vector2<float> scale_dpi_factor();

        ds::dimensions<int32_t> screen_size();
        ds::dimensions<int32_t> render_size();

        ds::position<float> position();
        ds::position<float> center();

    public:
        rl::Window& operator=(Window window) = delete;
        rl::Window& operator=(Window& window) = delete;
        rl::Window& operator=(Window&& window) = delete;
        rl::Window& operator=(const Window& window) = delete;

    protected:
        bool setup(int32_t width = 1920, int32_t height = 1080, std::string title = "roguelite");
        bool teardown();
    };
}
