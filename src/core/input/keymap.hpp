#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <raylib.h>
#include <simdjson.h>
#include <string_view>

namespace rl::input
{
    enum class Key : std::underlying_type_t<::KeyboardKey>
    {
        Null = KEY_NULL,                   // Key: NULL, used for no key pressed
        Apostrophe = KEY_APOSTROPHE,       // Key: '
        Comma = KEY_COMMA,                 // Key: ,
        Minus = KEY_MINUS,                 // Key: -
        Period = KEY_PERIOD,               // Key: .
        ForwardSlash = KEY_SLASH,          // Key: /
        Zero = KEY_ZERO,                   // Key: 0
        One = KEY_ONE,                     // Key: 1
        Two = KEY_TWO,                     // Key: 2
        Three = KEY_THREE,                 // Key: 3
        Four = KEY_FOUR,                   // Key: 4
        Five = KEY_FIVE,                   // Key: 5
        Six = KEY_SIX,                     // Key: 6
        Seven = KEY_SEVEN,                 // Key: 7
        Eight = KEY_EIGHT,                 // Key: 8
        Nine = KEY_NINE,                   // Key: 9
        Semicolon = KEY_SEMICOLON,         // Key: ;
        Equal = KEY_EQUAL,                 // Key: =
        A = KEY_A,                         // Key: A | a
        B = KEY_B,                         // Key: B | b
        C = KEY_C,                         // Key: C | c
        D = KEY_D,                         // Key: D | d
        E = KEY_E,                         // Key: E | e
        F = KEY_F,                         // Key: F | f
        G = KEY_G,                         // Key: G | g
        H = KEY_H,                         // Key: H | h
        I = KEY_I,                         // Key: I | i
        J = KEY_J,                         // Key: J | j
        K = KEY_K,                         // Key: K | k
        L = KEY_L,                         // Key: L | l
        M = KEY_M,                         // Key: M | m
        N = KEY_N,                         // Key: N | n
        O = KEY_O,                         // Key: O | o
        P = KEY_P,                         // Key: P | p
        Q = KEY_Q,                         // Key: Q | q
        R = KEY_R,                         // Key: R | r
        S = KEY_S,                         // Key: S | s
        T = KEY_T,                         // Key: T | t
        U = KEY_U,                         // Key: U | u
        V = KEY_V,                         // Key: V | v
        W = KEY_W,                         // Key: W | w
        X = KEY_X,                         // Key: X | x
        Y = KEY_Y,                         // Key: Y | y
        Z = KEY_Z,                         // Key: Z | z
        LeftBracket = KEY_LEFT_BRACKET,    // Key: [
        Backslash = KEY_BACKSLASH,         // Key: '\'
        RightBracket = KEY_RIGHT_BRACKET,  // Key: ]
        Tilda = KEY_GRAVE,                 // Key: `
        Space = KEY_SPACE,                 // Key: Space
        Escape = KEY_ESCAPE,               // Key: Esc
        Enter = KEY_ENTER,                 // Key: Enter
        Tab = KEY_TAB,                     // Key: Tab
        Backspace = KEY_BACKSPACE,         // Key: Backspace
        Insert = KEY_INSERT,               // Key: Ins
        Delete = KEY_DELETE,               // Key: Del
        Right = KEY_RIGHT,                 // Key: Cursor right
        Left = KEY_LEFT,                   // Key: Cursor left
        Down = KEY_DOWN,                   // Key: Cursor down
        Up = KEY_UP,                       // Key: Cursor up
        PageUp = KEY_PAGE_UP,              // Key: Page up
        PageDown = KEY_PAGE_DOWN,          // Key: Page down
        Home = KEY_HOME,                   // Key: Home
        End = KEY_END,                     // Key: End
        CapsLock = KEY_CAPS_LOCK,          // Key: Caps lock
        ScrollLock = KEY_SCROLL_LOCK,      // Key: Scroll down
        NumLock = KEY_NUM_LOCK,            // Key: Num lock
        PrintScreen = KEY_PRINT_SCREEN,    // Key: Print screen
        Pause = KEY_PAUSE,                 // Key: Pause
        F1 = KEY_F1,                       // Key: F1
        F2 = KEY_F2,                       // Key: F2
        F3 = KEY_F3,                       // Key: F3
        F4 = KEY_F4,                       // Key: F4
        F5 = KEY_F5,                       // Key: F5
        F6 = KEY_F6,                       // Key: F6
        F7 = KEY_F7,                       // Key: F7
        F8 = KEY_F8,                       // Key: F8
        F9 = KEY_F9,                       // Key: F9
        F10 = KEY_F10,                     // Key: F10
        F11 = KEY_F11,                     // Key: F11
        F12 = KEY_F12,                     // Key: F12
        LeftShift = KEY_LEFT_SHIFT,        // Key: Shift left
        LeftCtrl = KEY_LEFT_CONTROL,       // Key: Control left
        LeftAlt = KEY_LEFT_ALT,            // Key: Alt left
        LeftSuper = KEY_LEFT_SUPER,        // Key: Super left
        RightShift = KEY_RIGHT_SHIFT,      // Key: Shift right
        RightCtrl = KEY_RIGHT_CONTROL,     // Key: Control right
        RightAlt = KEY_RIGHT_ALT,          // Key: Alt right
        RightSuper = KEY_RIGHT_SUPER,      // Key: Super right
        KB_Menu = KEY_KB_MENU,             // Key: KB menu
        NP_0 = KEY_KP_0,                   // Key: Keypad 0
        NP_1 = KEY_KP_1,                   // Key: Keypad 1
        NP_2 = KEY_KP_2,                   // Key: Keypad 2
        NP_3 = KEY_KP_3,                   // Key: Keypad 3
        NP_4 = KEY_KP_4,                   // Key: Keypad 4
        NP_5 = KEY_KP_5,                   // Key: Keypad 5
        NP_6 = KEY_KP_6,                   // Key: Keypad 6
        NP_7 = KEY_KP_7,                   // Key: Keypad 7
        NP_8 = KEY_KP_8,                   // Key: Keypad 8
        NP_9 = KEY_KP_9,                   // Key: Keypad 9
        NP_Decimal = KEY_KP_DECIMAL,       // Key: Keypad .
        NP_Divide = KEY_KP_DIVIDE,         // Key: Keypad /
        NP_MULTIPLY = KEY_KP_MULTIPLY,     // Key: Keypad *
        NP_Subtract = KEY_KP_SUBTRACT,     // Key: Keypad -
        NP_Add = KEY_KP_ADD,               // Key: Keypad +
        NP_Enter = KEY_KP_ENTER,           // Key: Keypad Enter
        NP_Equal = KEY_KP_EQUAL,           // Key: Keypad =
    };

    enum class InputDevice
    {
        None,
        Unknown,
        Mouse,
        Keyboard,
        Gamepad
    };

    enum class ActionCategory
    {
        None,
        UI,
        Game
    };

    enum class GameplayAction
    {
        None,
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        RotateUp,
        RotateDown,
        RotateLeft,
        RotateRight,
        Dash,
        Shoot,
        UseItem,
        PrevWeapon,
        NextWeapon,
        ToggleDebugInfo,
    };

    enum class UIAction
    {
        Up,
        Down,
        Left,
        Right,
        Next,
        Prev,
        Accept,
        Cancel,
    };

    struct ActionInput
    {
        std::string action;
        Key button{ KEY_NULL };
        bool modifier_ctrl{ false };
        bool modifier_shift{ false };
        InputDevice device{ InputDevice::None };
        ActionCategory category{ ActionCategory::None };
    };

    struct Keymap
    {
        inline static std::filesystem::path keymap_cfg{ "./data/configs/keymap.json" };

        const static inline std::array InputDeviceMap =
            std::to_array<std::pair<std::string_view, InputDevice>>({
                { "None", InputDevice::None },
                { "Mouse", InputDevice::Mouse },
                { "Keyboard", InputDevice::Keyboard },
                { "Gamepad", InputDevice::Gamepad },
            });

        const static inline std::array UIActionMap =
            std::to_array<std::pair<std::string_view, UIAction>>({
                { "navigate_up", UIAction::Up },
                { "navigate_down", UIAction::Down },
                { "navigate_left", UIAction::Left },
                { "navigate_right", UIAction::Right },
                { "accept", UIAction::Accept },
                { "cancel", UIAction::Cancel },
            });

        const static inline std::array GameActionMap =
            std::to_array<std::pair<std::string_view, GameplayAction>>({
                { "move_up", GameplayAction::MoveUp },
                { "move_down", GameplayAction::MoveDown },
                { "move_left", GameplayAction::MoveLeft },
                { "move_right", GameplayAction::MoveRight },
                { "dash", GameplayAction::Dash },
                { "shoot", GameplayAction::Shoot },
                { "use_item", GameplayAction::UseItem },
                { "prev_weapon", GameplayAction::PrevWeapon },
                { "next_weapon", GameplayAction::NextWeapon },
                { "debug_mode", GameplayAction::ToggleDebugInfo },
            });

        const static inline std::array KeyboardButtonMap =
            std::to_array<std::pair<std::string_view, Key>>({
                { "A", Key::A },
                { "B", Key::B },
                { "C", Key::C },
                { "D", Key::D },
                { "E", Key::E },
                { "F", Key::F },
                { "G", Key::G },
                { "H", Key::H },
                { "I", Key::I },
                { "J", Key::J },
                { "K", Key::K },
                { "L", Key::L },
                { "M", Key::M },
                { "N", Key::N },
                { "O", Key::O },
                { "P", Key::P },
                { "Q", Key::Q },
                { "R", Key::R },
                { "S", Key::S },
                { "T", Key::T },
                { "U", Key::U },
                { "V", Key::V },
                { "W", Key::W },
                { "X", Key::X },
                { "Y", Key::Y },
                { "Z", Key::Z },

                { "1", Key::One },
                { "2", Key::Two },
                { "3", Key::Three },
                { "4", Key::Four },
                { "5", Key::Five },
                { "6", Key::Six },
                { "7", Key::Seven },
                { "8", Key::Eight },
                { "9", Key::Nine },
                { "0", Key::Zero },

                { "F1", Key::F1 },
                { "F2", Key::F2 },
                { "F3", Key::F3 },
                { "F4", Key::F4 },
                { "F5", Key::F5 },
                { "F6", Key::F6 },
                { "F7", Key::F7 },
                { "F8", Key::F8 },
                { "F9", Key::F9 },
                { "F10", Key::F10 },
                { "F11", Key::F11 },
                { "F12", Key::F12 },

                { "'", Key::Apostrophe },
                { ",", Key::Comma },
                { "-", Key::Minus },
                { ".", Key::Period },
                { "/", Key::ForwardSlash },
                { ";", Key::Semicolon },
                { "=", Key::Equal },

                { "Space", Key::Space },
                { "Escape", Key::Escape },
                { "Enter", Key::Enter },
                { "Tab", Key::Tab },
                { "Backspace", Key::Backspace },
                { "Insert", Key::Insert },
                { "Delete", Key::Delete },

                { "LShift", Key::LeftShift },
                { "RShift", Key::RightShift },
                { "LCtrl", Key::LeftCtrl },
                { "RCtrl", Key::RightCtrl },
                { "LCtrl", Key::LeftAlt },
                { "RCtrl", Key::RightAlt },

                { "Up", Key::Up },
                { "Down", Key::Down },
                { "Left", Key::Left },
                { "Right", Key::Right },

                { "NPAdd", Key::NP_Add },
                { "NPSubtract", Key::NP_Subtract },
            });

        Keymap()
        {
            simdjson::dom::parser config_parser{};
            simdjson::dom::element keymap{ config_parser.load(keymap_cfg.string()) };

            auto game_actions = keymap["game"];
            for (const auto& actions : GameActionMap)
            {
                const auto& [action_label, action_type] = actions;
                auto&& curr_action_keys = game_actions[action_label];
                for (const auto&& action_keys : curr_action_keys)
                {
                    ActionInput action = {
                        .action = std::string{ action_label },
                        .button = get_button(std::string{ action_keys["button"] }),
                        .modifier_ctrl = false,
                        .modifier_shift = false,
                        .device = get_device(std::string{ action_keys["device"] }),
                        .category = ActionCategory::Game,
                    };

                    game_input_mappings.push_back(std::move(action));
                }
            }
        }

    private:
        static Key get_button(std::string label)
        {
            for (const auto& mapping : KeyboardButtonMap)
            {
                auto&& [button_label, button_code] = mapping;
                if (label == button_label)
                    return button_code;
            }
            return Key::Null;
        }

        static InputDevice get_device(std::string label)
        {
            for (const auto& mapping : InputDeviceMap)
            {
                auto&& [button_label, device] = mapping;
                if (label == button_label)
                    return device;
            }
            return InputDevice::Unknown;
        }

        std::vector<ActionInput> ui_input_mappings{};
        std::vector<ActionInput> game_input_mappings{};
    };
}
