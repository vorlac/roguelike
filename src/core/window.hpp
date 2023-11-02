#pragma once

#include <string>
#include <vector>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/gui.hpp"

namespace rl
{
    class Window
    {
    public:
        Window();
        Window(ds::dimensions<i32> dimensions, std::string title);
        ~Window();

        constexpr inline void render(auto render_func)
        {
            this->begin_drawing();
            render_func();
            this->update_gui();
            this->end_drawing();
        }

        void begin_drawing();
        void update_gui();
        void end_drawing(bool draw_fps = true);

        bool should_close() const;
        void close() const;

        bool is_ready();
        bool is_hidden();
        bool is_fullscreen();
        bool is_minimized();
        bool is_maximized();
        bool is_focused();
        bool is_resized();

        bool get_state(u32 flag);
        void set_state(u32 flags);
        void clear_state(u32 flags);

        void toggle_fullscreen();
        void maximize();
        void minimize();
        void restore();

        void set_icon(raylib::Image&& image);
        void set_icons(std::vector<raylib::Image>&& images);

        void title(std::string title);

        void set_position(ds::point<i32> pos);
        void set_monitor(i16 monitor);
        void min_size(ds::dimensions<i32> min_size);
        void size(ds::dimensions<i32> size);
        void opacity(f32 opacity);

        void* handle();

        ds::vector2<f32> scale_dpi_factor();
        ds::dimensions<i32> screen_size();
        ds::dimensions<i32> render_size();

        ds::point<f32> position();
        ds::point<f32> center();

    public:
        rl::Window& operator=(Window window)        = delete;
        rl::Window& operator=(Window& window)       = delete;
        rl::Window& operator=(Window&& window)      = delete;
        rl::Window& operator=(const Window& window) = delete;

    protected:
        bool setup(i32 width = 1920, i32 height = 1080, std::string title = "roguelite");
        bool teardown();

    private:
        rl::GUI m_gui{};
        rl::Input m_input{};
    };
}
