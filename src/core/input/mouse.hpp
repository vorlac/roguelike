#pragma once

#include <cstdint>
#include <type_traits>

#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::input::device
{
    class Mouse
    {
    public:
        using ButtonID = std::underlying_type_t<raylib::MouseButton>;
        using CursorID = std::underlying_type_t<raylib::MouseCursor>;

    public:
        bool is_button_up(ButtonID button) const;
        bool is_button_down(ButtonID button) const;
        bool is_button_pressed(ButtonID button) const;
        bool is_button_released(ButtonID button) const;

        void set_x(int32_t x) const;
        void set_y(int32_t y) const;
        void set_position(int32_t x, int32_t y) const;
        void set_position(ds::position<int32_t> pos) const;
        void set_offset(int32_t x_offset = 0, int32_t y_offset = 0) const;
        void set_offset(ds::vector2<int32_t> offset) const;
        void set_scale(float x_scale = 1.0f, float y_scale = 1.0f) const;
        void set_scale(ds::vector2<float> scale) const;
        void set_cursor(CursorID cursor = raylib::MouseCursor::MOUSE_CURSOR_DEFAULT) const;

        int32_t get_x() const;
        int32_t get_y() const;

        ds::position<int32_t> get_position() const;
        ds::vector2<int32_t> get_delta() const;
        ds::vector2<float> get_wheel_move_v() const;

        float get_wheel_move() const;
    };
}
