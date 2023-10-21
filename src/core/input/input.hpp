#pragma once

#include <fmt/format.h>

#include "core/input/gamepad.hpp"
#include "core/input/keyboard.hpp"
#include "core/input/keymap.hpp"
#include "core/input/mouse.hpp"
#include "core/math.hpp"

namespace rl::input
{
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
            m_active_game_actions.reserve(64);
            m_active_ui_actions.reserve(64);
        }

        std::vector<GameplayAction> active_game_actions()
        {
            m_active_game_actions.clear();

            const auto& game_actions = m_keymap.game_action_keymap();
            for (const auto& action : game_actions)
            {
                auto button = std::to_underlying(action.button);
                switch (action.device)
                {
                    case InputDevice::Keyboard:
                        if (raylib::IsKeyDown(button))
                            m_active_game_actions.push_back(action.action);
                }
            }

            return m_active_game_actions;
        }

        std::vector<UIAction> active_ui_actions()
        {
            m_active_ui_actions.clear();

            const auto& game_actions = m_keymap.ui_action_keymap();
            for (const auto& action : game_actions)
            {
                auto button = std::to_underlying(action.button);
                switch (action.device)
                {
                    case InputDevice::Keyboard:
                        if (raylib::IsKeyDown(button))
                            m_active_ui_actions.push_back(action.action);
                }
            }

            return m_active_ui_actions;
        }

    private:
        ds::vector2<float> get_vector() const
        {
            device::Gamepad::AxisID pos_x = 0;
            device::Gamepad::AxisID neg_x = 1;
            device::Gamepad::AxisID pos_y = 2;
            device::Gamepad::AxisID neg_y = 3;
            float deadzone = 0.1f;

            ds::vector2<float> vec = {
                m_gamepad.get_axis_movement(pos_x) - m_gamepad.get_axis_movement(neg_x),
                m_gamepad.get_axis_movement(pos_y) - m_gamepad.get_axis_movement(neg_y),
            };

            float length = vec.length();
            if (length <= deadzone)
                return { 0.0f, 0.0f };
            else if (length > 1.0f)
                return { vec / length };
            else
                return vec * (rl::math::inverse_lerp(deadzone, 1.0f, length) / length);
        }

    private:
        input::device::Mouse m_mouse{};
        input::device::Keyboard m_keyboard{};
        input::device::Gamepad m_gamepad{};

        input::Keymap m_keymap{};
        std::vector<GameplayAction> m_active_game_actions{};
        std::vector<UIAction> m_active_ui_actions{};
    };
}
