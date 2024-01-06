#include <array>

#include "core/assert.hpp"
#include "core/mouse.hpp"
#include "utils/conversions.hpp"

namespace rl {
    void Mouse::process_button_down(const Mouse::Button::type mouse_button)
    {
        runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");

        // store it as held
        m_buttons_held |= m_buttons_pressed;
        // clear it from pressed
        m_buttons_pressed |= SDL_BUTTON(mouse_button);
        // clear it from released
        m_buttons_released = 0;  //&= ~SDL_BUTTON(mouse_button);
    }

    void Mouse::process_button_up(const Mouse::Button::type mouse_button)
    {
        runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");

        // set in released buttons
        m_buttons_released |= SDL_BUTTON(mouse_button);
        // clear from pressed buttons
        m_buttons_pressed &= ~SDL_BUTTON(mouse_button);
        // clear from held buttons
        m_buttons_held &= ~SDL_BUTTON(mouse_button);
    }

    void Mouse::process_motion(const Event::Data::Motion& motion)
    {
        m_prev_cursor_pos = m_cursor_position;
        m_cursor_position = {
            static_cast<i32>(motion.x),
            static_cast<i32>(motion.y),
        };
    }

    void Mouse::process_wheel(const Mouse::Event::Data::Wheel& wheel)
    {
        m_prev_wheel_pos = m_wheel_position;
        auto new_wheel_pos{ wheel };
        if (new_wheel_pos.direction == Mouse::Wheel::Direction::Flipped)
        {
            new_wheel_pos.x *= -1;
            new_wheel_pos.y *= -1;
        }

        if (new_wheel_pos.x != 0)
        {
            // positive to the right and negative to the left
            m_wheel_position.x += static_cast<i32>(new_wheel_pos.x * 10.0f);
        }

        if (new_wheel_pos.y != 0)
        {
            // positive away from the user and negative towards the user
            m_wheel_position.y -= static_cast<i32>(new_wheel_pos.y * 10.0f);
        }
    }

    ds::point<i32> Mouse::pos() const
    {
        return m_cursor_position;
    }

    ds::vector2<i32> Mouse::wheel() const
    {
        return m_wheel_position;
    }

    ds::vector2<i32> Mouse::pos_delta() const
    {
        return m_cursor_position - m_prev_cursor_pos;
    }

    Mouse::Button::type Mouse::button_pressed() const
    {
        return Mouse::Button::type(m_buttons_pressed);
    }

    Mouse::Button::type Mouse::button_released() const
    {
        return Mouse::Button::type(m_buttons_released);
    }

    bool Mouse::is_button_down(Mouse::Button::type button) const
    {
        return this->is_button_pressed(button) || this->is_button_held(button);
    }

    bool Mouse::is_button_pressed(Mouse::Button::type button) const
    {
        return 0 != (m_buttons_pressed & SDL_BUTTON(button));
    }

    bool Mouse::is_button_released(Mouse::Button::type button) const
    {
        return 0 != (m_buttons_released & SDL_BUTTON(button));
    }

    bool Mouse::is_button_held(Mouse::Button::type button) const
    {
        return 0 != (m_buttons_held & SDL_BUTTON(button));
    }

    bool Mouse::all_buttons_down(std::vector<Mouse::Button::type> buttons) const
    {
        bool ret{ true };

        for (auto&& button : buttons)
            ret &= this->is_button_down(button);

        return ret;
    }

    bool Mouse::any_buttons_down(std::vector<Mouse::Button::type> buttons) const
    {
        for (auto&& button : buttons)
            if (this->is_button_down(button))
                return true;

        return false;
    }
}
