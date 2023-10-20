#include "core/input/keyboard.hpp"

namespace rl::input::device
{
    bool Keyboard::is_key_pressed(Keyboard::ButtonID key) const
    {
        return raylib::IsKeyPressed(key);
    }

    bool Keyboard::is_key_down(Keyboard::ButtonID key) const
    {
        return raylib::IsKeyDown(key);
    }

    bool Keyboard::is_key_released(Keyboard::ButtonID key) const
    {
        return raylib::IsKeyReleased(key);
    }

    bool Keyboard::is_key_up(Keyboard::ButtonID key) const
    {
        return raylib::IsKeyUp(key);
    }

    Keyboard::ButtonID Keyboard::get_key_pressed() const
    {
        // Get key pressed (keycode), call it multiple times for
        // keys queued, returns 0 when the queue is empty
        return raylib::GetKeyPressed();
    }

    Keyboard::ButtonID Keyboard::get_char_pressed() const
    {
        // Get char pressed (unicode), call it multiple times for
        // chars queued, returns 0 when the queue is empty
        return raylib::GetCharPressed();
    }

    void Keyboard::set_exit_key(Keyboard::ButtonID key) const
    {
        return raylib::SetExitKey(key);
    }
}
