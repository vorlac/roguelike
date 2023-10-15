#include "core/input/gamepad.hpp"

namespace rl::input::device
{
    Gamepad::Gamepad(int32_t id)
        : m_gamepad_id(id)
    {
    }

    bool Gamepad::is_available() const
    {
        return ::IsGamepadAvailable(m_gamepad_id);
    }

    bool Gamepad::is_button_pressed(Gamepad::ButtonID button) const
    {
        return ::IsGamepadButtonPressed(m_gamepad_id, button);
    }

    bool Gamepad::is_button_down(Gamepad::ButtonID button) const
    {
        return ::IsGamepadButtonDown(m_gamepad_id, button);
    }

    bool Gamepad::is_button_released(Gamepad::ButtonID button) const
    {
        return ::IsGamepadButtonReleased(m_gamepad_id, button);
    }

    bool Gamepad::is_button_up(Gamepad::ButtonID button) const
    {
        return ::IsGamepadButtonUp(m_gamepad_id, button);
    }

    float Gamepad::get_axis_movement(Gamepad::AxisID axis) const
    {
        return ::GetGamepadAxisMovement(m_gamepad_id, axis);
    }

    Gamepad::ButtonID Gamepad::get_button_pressed() const
    {
        return ::GetGamepadButtonPressed();
    }

    std::string Gamepad::get_name() const
    {
        return ::GetGamepadName(m_gamepad_id);
    }

    Gamepad::ButtonID Gamepad::get_axis_count() const
    {
        return ::GetGamepadAxisCount(m_gamepad_id);
    }

    Gamepad::ButtonID Gamepad::set_mappings(const std::string& mappings) const
    {
        return ::SetGamepadMappings(mappings.c_str());
    }
}
