#include "core/input/mouse.hpp"

#include "core/utils/conversions.hpp"

namespace rl::input
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
            cast::to<i32>(pos.x),
            cast::to<i32>(pos.y),
        };
    }

    ds::vector2<i32> Mouse::get_delta() const
    {
        auto delta{ raylib::GetMouseDelta() };
        return {
            cast::to<i32>(delta.x),
            cast::to<i32>(delta.y),
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

    void Mouse::set_scale(f32 x_scale, f32 y_scale) const
    {
        return raylib::SetMouseScale(x_scale, y_scale);
    }

    void Mouse::set_scale(ds::vector2<f32> scale) const
    {
        return raylib::SetMouseScale(scale.x, scale.y);
    }

    float Mouse::get_wheel_move() const
    {
        return raylib::GetMouseWheelMove();
    }

    ds::vector2<f32> Mouse::get_wheel_move_v() const
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

    void Mouse::hide_cursor() const
    {
        return raylib::HideCursor();
    }

    void Mouse::show_cursor() const
    {
        return raylib::ShowCursor();
    }
}
