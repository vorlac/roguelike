#pragma once

#include <concepts>
#include <type_traits>

#include "core/input/keymap.hpp"
#include "core/numeric_types.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::input
{
    class Keyboard
    {
    public:
        using ButtonID = std::underlying_type_t<raylib::KeyboardKey>;

    public:
        constexpr inline bool is_key_down(auto key) const
        {
            return raylib::IsKeyDown(static_cast<Keyboard::ButtonID>(key));
        }

        constexpr inline bool is_key_pressed(auto key) const
        {
            return raylib::IsKeyPressed(static_cast<Keyboard::ButtonID>(key));
        }

        constexpr inline bool is_key_released(auto key) const
        {
            return raylib::IsKeyReleased(static_cast<Keyboard::ButtonID>(key));
        }

        constexpr inline bool is_key_up(auto key) const
        {
            return raylib::IsKeyUp(static_cast<Keyboard::ButtonID>(key));
        }

        constexpr inline void set_exit_key(auto key) const
        {
            return raylib::SetExitKey(static_cast<Keyboard::ButtonID>(key));
        }

        template <typename T = Key>
            requires std::same_as<T, i32> || std::same_as<T, ButtonID> || std::same_as<T, Key>
        constexpr inline T get_key_pressed() const
        {
            // Get key pressed (keycode), call it multiple times for
            // keys queued, returns 0 when the queue is empty
            return static_cast<T>(raylib::GetKeyPressed());
        }

        template <typename T = u32>
            requires std::same_as<T, u32>
        constexpr inline T get_char_pressed() const
        {
            // Get char pressed (unicode), call it multiple times for
            // chars queued, returns 0 when the queue is empty
            return static_cast<T>(raylib::GetCharPressed());
        }

        constexpr inline bool is_ctrl_down() const
        {
            return this->is_key_down(input::Key::LeftCtrl) ||
                   this->is_key_down(input::Key::RightCtrl);
        }

        constexpr inline bool is_shift_down() const
        {
            return this->is_key_down(input::Key::LeftShift) ||
                   this->is_key_down(input::Key::RightShift);
        }

        constexpr inline bool is_alt_down() const
        {
            return this->is_key_down(input::Key::RightAlt) ||  //
                   this->is_key_down(input::Key::LeftAlt);
        }

        constexpr inline bool is_super_down() const
        {
            return this->is_key_down(input::Key::LeftSuper) ||
                   this->is_key_down(input::Key::RightSuper);
        };
    };
}
