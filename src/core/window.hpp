#pragma once

#include <string>
#include <vector>

#include "core/display.hpp"
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

        void update_gui();

        void begin_drawing() const;
        void end_drawing(bool draw_fps = true) const;

        bool should_close() const;
        void close() const;

        bool is_ready() const;
        bool is_hidden() const;
        bool is_fullscreen() const;
        bool is_minimized() const;
        bool is_maximized() const;
        bool is_focused() const;
        bool is_resized() const;

        bool get_state(u32 flag) const;
        void set_state(u32 flags) const;
        void clear_state(u32 flags) const;

        void toggle_fullscreen() const;
        void maximize() const;
        void minimize() const;
        void restore() const;

        void set_icon(raylib::Image&& image) const;
        void set_icons(std::vector<raylib::Image>&& images) const;

        void title(std::string title) const;

        void set_position(ds::point<i32> pos) const;
        void set_monitor(i16 monitor) const;
        void min_size(ds::dimensions<i32> min_size) const;
        void size(ds::dimensions<i32> size) const;
        void opacity(f32 opacity) const;

        void* handle() const;

        f32 frame_time() const;
        ds::vector2<f32> scale_dpi_factor() const;
        ds::dimensions<i32> screen_size() const;
        ds::dimensions<i32> render_size() const;

        ds::point<f32> position() const;
        ds::point<f32> center() const;

    public:
        rl::Window& operator=(Window window)        = delete;
        rl::Window& operator=(Window& window)       = delete;
        rl::Window& operator=(Window&& window)      = delete;
        rl::Window& operator=(const Window& window) = delete;

    protected:
        bool setup(i32 width = 1920, i32 height = 1080, std::string title = "roguelite");
        bool teardown();

    private:
        rl::gui m_gui{};
        rl::Input m_input{};
        rl::Display m_display{};
    };
}
