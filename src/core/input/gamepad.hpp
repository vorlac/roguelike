#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include "core/numeric_types.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::input::device
{
    class Gamepad
    {
    public:
        using AxisID   = std::underlying_type_t<raylib::GamepadAxis>;
        using ButtonID = std::underlying_type_t<raylib::GamepadButton>;

    public:
        Gamepad(i32 id = 0);

        bool is_button_pressed(Gamepad::ButtonID button) const;
        bool is_button_down(Gamepad::ButtonID button) const;
        bool is_button_released(Gamepad::ButtonID button) const;
        bool is_button_up(Gamepad::ButtonID button) const;
        bool is_available() const;

        float get_axis_movement(Gamepad::AxisID axis) const;
        Gamepad::ButtonID get_button_pressed() const;

        std::string get_name() const;
        i32 get_axis_count() const;
        i32 set_mappings(const std::string& mappings) const;

    protected:
        Gamepad::ButtonID m_gamepad_id{ 0 };
    };
}
