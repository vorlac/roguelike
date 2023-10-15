#pragma once

#include "core/input/gamepad.hpp"
#include "core/input/keyboard.hpp"
#include "core/input/mouse.hpp"

namespace rl::input
{
    enum class InputDevice
    {
        None,
        Mouse,
        Keyboard,
        Gamepad
    };

    struct ButtonInfo
    {
        uint32_t id{ 0 };
        std::string_view description{};
    };

    struct ButtonState
    {
        bool pressed{ false };
        bool released{ false };
        bool held{ false };
        int32_t axis{ 9999 };
    };

    struct InputEvent
    {
        InputDevice device{ InputDevice::None };
        ButtonInfo button{};
    };

    class Input
    {
    public:
        Input()
        {
        }

    private:
        input::device::Mouse m_mouse{};
        input::device::Keyboard m_keyboard{};
        input::device::Gamepad m_gamepad{};
    };
}
