#include "core/mouse.hpp"

namespace rl {
    void Mouse::process_button_down(Mouse::Button::type mouse_button)
    {
        runtime_assert((mouse_button - 1) < Mouse::Button::Count, "invalid mouse button");
        switch (mouse_button)
        {
            case Mouse::Button::Left:
                [[fallthrough]];
            case Mouse::Button::Middle:
                [[fallthrough]];
            case Mouse::Button::Right:
                [[fallthrough]];
            case Mouse::Button::X1:
                [[fallthrough]];
            case Mouse::Button::X2:
                m_button_states |= (1 << (mouse_button - 1));
                break;
        }
    }

    void Mouse::process_button_up(Mouse::Button::type mouse_button)
    {
        runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");
        m_button_states &= ~(1 << (mouse_button - 1));
    }

    void Mouse::process_motion(Event::Data::Motion& motion)
    {
        m_prev_cursor_pos = m_cursor_position;
        m_cursor_position = { motion.x, motion.y };
    }

    void Mouse::process_wheel(Mouse::Event::Data::Wheel& wheel)
    {
        m_prev_wheel_pos = m_wheel_position;
        if (wheel.direction == Mouse::Wheel::Direction::Flipped)
        {
            wheel.x *= -1;
            wheel.y *= -1;
        }

        if (wheel.x != 0.0f)
        {
            // positive to the right and negative to the left
            m_wheel_position.x += wheel.x * 10.0f;
        }

        if (wheel.x != 0.0f)
        {
            // positive away from the user and negative towards the user
            m_wheel_position.y -= wheel.x * 10.0f;
        }
    }

    ds::point<f32> Mouse::pos() const
    {
        return m_cursor_position;
    }

    ds::vector2<f32> Mouse::wheel() const
    {
        return m_wheel_position;
    }

    ds::point<f32> Mouse::pos_delta() const
    {
        return m_prev_cursor_pos - m_cursor_position;
    }

    bool Mouse::is_button_down(u32 button) const
    {
        return 0 != (m_button_states & SDL_BUTTON(button));
    }
}
