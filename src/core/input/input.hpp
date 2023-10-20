#pragma once

#include <fmt/format.h>

#include "core/input/gamepad.hpp"
#include "core/input/keyboard.hpp"
#include "core/input/keymap.hpp"
#include "core/input/mouse.hpp"

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
                        if (::IsKeyDown(button))
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
                        if (::IsKeyDown(button))
                            m_active_ui_actions.push_back(action.action);
                }
            }

            return m_active_ui_actions;
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
