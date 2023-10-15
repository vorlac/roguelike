#pragma once

#include <cstdint>
#include <raylib.h>
#include <type_traits>

namespace rl::input::device
{
    class Keyboard
    {
    public:
        using ButtonID = std::underlying_type_t<::KeyboardKey>;

    public:
        bool is_key_up(ButtonID key) const;
        bool is_key_down(ButtonID key) const;
        bool is_key_pressed(ButtonID key) const;
        bool is_key_released(ButtonID key) const;

        ButtonID get_key_pressed() const;
        ButtonID get_char_pressed() const;

        void set_exit_key(ButtonID key) const;
    };
}
