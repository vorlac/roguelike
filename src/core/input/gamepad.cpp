#include "core/input/gamepad.hpp"

namespace rl::input::device
{
    Gamepad::Gamepad(int32_t id)
        : m_gamepad_id(id)
    {
    }

    bool Gamepad::is_available() const
    {
        return raylib::IsGamepadAvailable(m_gamepad_id);
    }

    bool Gamepad::is_button_pressed(Gamepad::ButtonID button) const
    {
        return raylib::IsGamepadButtonPressed(m_gamepad_id, button);
    }

    bool Gamepad::is_button_down(Gamepad::ButtonID button) const
    {
        return raylib::IsGamepadButtonDown(m_gamepad_id, button);
    }

    bool Gamepad::is_button_released(Gamepad::ButtonID button) const
    {
        return raylib::IsGamepadButtonReleased(m_gamepad_id, button);
    }

    bool Gamepad::is_button_up(Gamepad::ButtonID button) const
    {
        return raylib::IsGamepadButtonUp(m_gamepad_id, button);
    }

    float Gamepad::get_axis_movement(Gamepad::AxisID axis) const
    {
        return raylib::GetGamepadAxisMovement(m_gamepad_id, axis);
    }

    Gamepad::ButtonID Gamepad::get_button_pressed() const
    {
        return raylib::GetGamepadButtonPressed();
    }

    std::string Gamepad::get_name() const
    {
        return raylib::GetGamepadName(m_gamepad_id);
    }

    Gamepad::ButtonID Gamepad::get_axis_count() const
    {
        return raylib::GetGamepadAxisCount(m_gamepad_id);
    }

    Gamepad::ButtonID Gamepad::set_mappings(const std::string& mappings) const
    {
        return raylib::SetGamepadMappings(mappings.c_str());
    }
}
