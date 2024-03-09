#include <array>

#include "core/assert.hpp"
#include "core/mouse.hpp"
#include "utils/logging.hpp"

namespace rl {
    Mouse::Mouse()
    {
        for (u32 i = Cursor::Arrow; i < Cursor::CursorCount; ++i)
            m_system_cursors[static_cast<std::size_t>(i)] = SDL3::SDL_CreateSystemCursor(
                static_cast<Cursor::type>(i));
    }

    Mouse::~Mouse()
    {
        for (u32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            if (m_system_cursors[i] != nullptr)
                SDL3::SDL_DestroyCursor(m_system_cursors[i]);
    }

    bool Mouse::set_cursor(const Mouse::Cursor::ID cursor_id) const
    {
        if (m_active_cursor == cursor_id)
            return false;

        runtime_assert(cursor_id < m_system_cursors.size(), "invalid cursor idx");
        SDL3::SDL_Cursor* cursor{ m_system_cursors[cursor_id] };
        runtime_assert(cursor != nullptr, "invalid cursor");

        const i32 result{ SDL3::SDL_SetCursor(cursor) };
        sdl_assert(result == 0, "failed to set cursor: {}", std::to_underlying(cursor_id));

        if (result == 0)
            m_active_cursor = cursor_id;
        return result == 0;
    }

    Mouse::Cursor::ID Mouse::active_cursor() const
    {
        return m_active_cursor;
    }

    void Mouse::process_button_down(const Mouse::Button::ID mouse_button)
    {
        runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");

        // store it as held
        m_buttons_held |= m_buttons_pressed;
        // clear it from pressed
        m_buttons_pressed |= SDL_BUTTON(mouse_button);
        // clear it from released
        m_buttons_released = 0;  //&= ~SDL_BUTTON(mouse_button);
    }

    void Mouse::process_button_up(const Mouse::Button::ID mouse_button)
    {
        runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");

        // set in released buttons
        m_buttons_released |= SDL_BUTTON(mouse_button);
        // clear from pressed buttons
        m_buttons_pressed &= ~SDL_BUTTON(mouse_button);
        // clear from held buttons
        m_buttons_held &= ~SDL_BUTTON(mouse_button);
    }

    void Mouse::process_motion_delta(const ds::vector2<f32>& delta)
    {
        m_prev_cursor_pos = m_cursor_position;
        m_cursor_position += delta;
    }

    void Mouse::process_motion(const Event::Data::Motion& motion)
    {
        m_prev_cursor_pos = m_cursor_position;
        m_cursor_position = {
            motion.x,
            motion.y,
        };
    }

    void Mouse::process_wheel(const Mouse::Event::Data::Wheel& wheel)
    {
        auto new_wheel_pos{ wheel };

        m_prev_wheel_pos = m_wheel_position;
        if (new_wheel_pos.direction == Mouse::Wheel::Direction::Flipped)
        {
            new_wheel_pos.x *= -1.0f;
            new_wheel_pos.y *= -1.0f;
        }

        if (new_wheel_pos.x != 0.0f)
            m_wheel_position.x += new_wheel_pos.x;
        if (new_wheel_pos.y != 0.0f)
            m_wheel_position.y -= new_wheel_pos.y;
    }

    ds::point<f32> Mouse::pos() const
    {
        return m_cursor_position;
    }

    ds::vector2<f32> Mouse::wheel() const
    {
        return m_wheel_position;
    }

    ds::vector2<f32> Mouse::pos_delta() const
    {
        return m_cursor_position - m_prev_cursor_pos;
    }

    ds::vector2<f32> Mouse::wheel_delta() const
    {
        return m_wheel_position - m_prev_wheel_pos;
    }

    Mouse::Button::ID Mouse::button_pressed() const
    {
        return static_cast<Mouse::Button::ID>(m_buttons_pressed);
    }

    Mouse::Button::ID Mouse::button_released() const
    {
        return static_cast<Mouse::Button::ID>(m_buttons_released);
    }

    bool Mouse::is_button_down(const Mouse::Button::ID button) const
    {
        return this->is_button_pressed(button) || this->is_button_held(button);
    }

    bool Mouse::is_button_pressed(const Mouse::Button::ID button) const
    {
        return 0 != (m_buttons_pressed & SDL_BUTTON(button));
    }

    bool Mouse::is_button_released(const Mouse::Button::ID button) const
    {
        return 0 != (m_buttons_released & SDL_BUTTON(button));
    }

    bool Mouse::is_button_held(const Mouse::Button::ID button) const
    {
        return 0 != (m_buttons_held & SDL_BUTTON(button));
    }

    bool Mouse::all_buttons_down(std::vector<Mouse::Button::ID>&& buttons) const
    {
        bool ret{ true };

        for (auto&& button : buttons)
            ret &= this->is_button_down(button);

        return ret;
    }

    bool Mouse::any_buttons_down(std::vector<Mouse::Button::ID>&& buttons) const
    {
        for (auto&& button : buttons)
        {
            if (this->is_button_down(button))
                return true;
        }

        return false;
    }

    std::string Mouse::name() const
    {
        return typeid(*this).name();
    }

    std::string Mouse::get_button_state(const Mouse::Button::ID button) const
    {
        return this->is_button_held(button)     ? "Held"
             : this->is_button_pressed(button)  ? "Pressed"
             : this->is_button_released(button) ? "Released"
                                                : "None";
    }
}
