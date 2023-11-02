#include "core/input/mouse.hpp"

namespace rl::input::device
{
    i32 Mouse::get_x() const
    {
        return raylib::GetMouseX();
    }

    i32 Mouse::get_y() const
    {
        return raylib::GetMouseY();
    }

    void Mouse::set_x(i32 x) const
    {
        return raylib::SetMousePosition(x, this->get_y());
    }

    void Mouse::set_y(i32 y) const
    {
        return raylib::SetMousePosition(this->get_x(), y);
    }

    ds::point<i32> Mouse::get_position() const
    {
        auto pos{ raylib::GetMousePosition() };
        return {
            static_cast<i32>(pos.x),
            static_cast<i32>(pos.y),
        };
    }

    void Mouse::set_position(i32 x, i32 y) const
    {
        return raylib::SetMousePosition(x, y);
    }

    void Mouse::set_position(ds::point<i32> pos) const
    {
        return raylib::SetMousePosition(pos.x, pos.y);
    }

    ds::vector2<i32> Mouse::get_delta() const
    {
        auto delta{ raylib::GetMouseDelta() };
        return {
            static_cast<i32>(delta.x),
            static_cast<i32>(delta.y),
        };
    }

    void Mouse::set_offset(i32 x_offset, i32 y_offset) const
    {
        return raylib::SetMouseOffset(x_offset, y_offset);
    }

    void Mouse::set_offset(ds::vector2<i32> offset) const
    {
        return raylib::SetMouseOffset(offset.x, offset.y);
    }

    void Mouse::set_scale(float x_scale, float y_scale) const
    {
        return raylib::SetMouseScale(x_scale, y_scale);
    }

    void Mouse::set_scale(ds::vector2<float> scale) const
    {
        return raylib::SetMouseScale(scale.x, scale.y);
    }

    float Mouse::get_wheel_move() const
    {
        return raylib::GetMouseWheelMove();
    }

    ds::vector2<float> Mouse::get_wheel_move_v() const
    {
        auto mov = raylib::GetMouseWheelMoveV();
        return {
            mov.x,
            mov.y,
        };
    }

    void Mouse::set_cursor(CursorID cursor) const
    {
        return raylib::SetMouseCursor(cursor);
    }
}
