#include "core/input/mouse.hpp"

namespace rl::input::device
{
    bool Mouse::is_button_pressed(ButtonID button) const
    {
        return raylib::IsMouseButtonPressed(button);
    }

    bool Mouse::is_button_down(ButtonID button) const
    {
        return raylib::IsMouseButtonDown(button);
    }

    bool Mouse::is_button_released(ButtonID button) const
    {
        return raylib::IsMouseButtonReleased(button);
    }

    bool Mouse::is_button_up(ButtonID button) const
    {
        return raylib::IsMouseButtonUp(button);
    }

    int32_t Mouse::get_x() const
    {
        return raylib::GetMouseX();
    }

    int32_t Mouse::get_y() const
    {
        return raylib::GetMouseY();
    }

    void Mouse::set_x(int32_t x) const
    {
        return raylib::SetMousePosition(x, this->get_y());
    }

    void Mouse::set_y(int32_t y) const
    {
        return raylib::SetMousePosition(this->get_x(), y);
    }

    ds::position<int32_t> Mouse::get_position() const
    {
        auto pos{ raylib::GetMousePosition() };
        return {
            static_cast<int32_t>(pos.x),
            static_cast<int32_t>(pos.y),
        };
    }

    void Mouse::set_position(int32_t x, int32_t y) const
    {
        return raylib::SetMousePosition(x, y);
    }

    void Mouse::set_position(ds::position<int32_t> pos) const
    {
        return raylib::SetMousePosition(pos.x, pos.y);
    }

    ds::vector2<int32_t> Mouse::get_delta() const
    {
        auto delta{ raylib::GetMouseDelta() };
        return {
            static_cast<int32_t>(delta.x),
            static_cast<int32_t>(delta.y),
        };
    }

    void Mouse::set_offset(int32_t x_offset, int32_t y_offset) const
    {
        return raylib::SetMouseOffset(x_offset, y_offset);
    }

    void Mouse::set_offset(ds::vector2<int32_t> offset) const
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
