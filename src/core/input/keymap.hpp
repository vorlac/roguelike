#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <simdjson.h>
#include <string_view>

#include "thirdparty/raylib.hpp"

namespace rl::input
{
    enum class Key : std::underlying_type_t<raylib::KeyboardKey>
    {
        Null = raylib::KEY_NULL,                   // Key: NULL, used for no key pressed
        Apostrophe = raylib::KEY_APOSTROPHE,       // Key: '
        Comma = raylib::KEY_COMMA,                 // Key: ,
        Minus = raylib::KEY_MINUS,                 // Key: -
        Period = raylib::KEY_PERIOD,               // Key: .
        ForwardSlash = raylib::KEY_SLASH,          // Key: /
        Zero = raylib::KEY_ZERO,                   // Key: 0
        One = raylib::KEY_ONE,                     // Key: 1
        Two = raylib::KEY_TWO,                     // Key: 2
        Three = raylib::KEY_THREE,                 // Key: 3
        Four = raylib::KEY_FOUR,                   // Key: 4
        Five = raylib::KEY_FIVE,                   // Key: 5
        Six = raylib::KEY_SIX,                     // Key: 6
        Seven = raylib::KEY_SEVEN,                 // Key: 7
        Eight = raylib::KEY_EIGHT,                 // Key: 8
        Nine = raylib::KEY_NINE,                   // Key: 9
        Semicolon = raylib::KEY_SEMICOLON,         // Key: ;
        Equal = raylib::KEY_EQUAL,                 // Key: =
        A = raylib::KEY_A,                         // Key: A | a
        B = raylib::KEY_B,                         // Key: B | b
        C = raylib::KEY_C,                         // Key: C | c
        D = raylib::KEY_D,                         // Key: D | d
        E = raylib::KEY_E,                         // Key: E | e
        F = raylib::KEY_F,                         // Key: F | f
        G = raylib::KEY_G,                         // Key: G | g
        H = raylib::KEY_H,                         // Key: H | h
        I = raylib::KEY_I,                         // Key: I | i
        J = raylib::KEY_J,                         // Key: J | j
        K = raylib::KEY_K,                         // Key: K | k
        L = raylib::KEY_L,                         // Key: L | l
        M = raylib::KEY_M,                         // Key: M | m
        N = raylib::KEY_N,                         // Key: N | n
        O = raylib::KEY_O,                         // Key: O | o
        P = raylib::KEY_P,                         // Key: P | p
        Q = raylib::KEY_Q,                         // Key: Q | q
        R = raylib::KEY_R,                         // Key: R | r
        S = raylib::KEY_S,                         // Key: S | s
        T = raylib::KEY_T,                         // Key: T | t
        U = raylib::KEY_U,                         // Key: U | u
        V = raylib::KEY_V,                         // Key: V | v
        W = raylib::KEY_W,                         // Key: W | w
        X = raylib::KEY_X,                         // Key: X | x
        Y = raylib::KEY_Y,                         // Key: Y | y
        Z = raylib::KEY_Z,                         // Key: Z | z
        LeftBracket = raylib::KEY_LEFT_BRACKET,    // Key: [
        Backslash = raylib::KEY_BACKSLASH,         // Key: '\'
        RightBracket = raylib::KEY_RIGHT_BRACKET,  // Key: ]
        Tilda = raylib::KEY_GRAVE,                 // Key: `
        Space = raylib::KEY_SPACE,                 // Key: Space
        Escape = raylib::KEY_ESCAPE,               // Key: Esc
        Enter = raylib::KEY_ENTER,                 // Key: Enter
        Tab = raylib::KEY_TAB,                     // Key: Tab
        Backspace = raylib::KEY_BACKSPACE,         // Key: Backspace
        Insert = raylib::KEY_INSERT,               // Key: Ins
        Delete = raylib::KEY_DELETE,               // Key: Del
        Right = raylib::KEY_RIGHT,                 // Key: Cursor right
        Left = raylib::KEY_LEFT,                   // Key: Cursor left
        Down = raylib::KEY_DOWN,                   // Key: Cursor down
        Up = raylib::KEY_UP,                       // Key: Cursor up
        PageUp = raylib::KEY_PAGE_UP,              // Key: Page up
        PageDown = raylib::KEY_PAGE_DOWN,          // Key: Page down
        Home = raylib::KEY_HOME,                   // Key: Home
        End = raylib::KEY_END,                     // Key: End
        CapsLock = raylib::KEY_CAPS_LOCK,          // Key: Caps lock
        ScrollLock = raylib::KEY_SCROLL_LOCK,      // Key: Scroll down
        NumLock = raylib::KEY_NUM_LOCK,            // Key: Num lock
        PrintScreen = raylib::KEY_PRINT_SCREEN,    // Key: Print screen
        Pause = raylib::KEY_PAUSE,                 // Key: Pause
        F1 = raylib::KEY_F1,                       // Key: F1
        F2 = raylib::KEY_F2,                       // Key: F2
        F3 = raylib::KEY_F3,                       // Key: F3
        F4 = raylib::KEY_F4,                       // Key: F4
        F5 = raylib::KEY_F5,                       // Key: F5
        F6 = raylib::KEY_F6,                       // Key: F6
        F7 = raylib::KEY_F7,                       // Key: F7
        F8 = raylib::KEY_F8,                       // Key: F8
        F9 = raylib::KEY_F9,                       // Key: F9
        F10 = raylib::KEY_F10,                     // Key: F10
        F11 = raylib::KEY_F11,                     // Key: F11
        F12 = raylib::KEY_F12,                     // Key: F12
        LeftShift = raylib::KEY_LEFT_SHIFT,        // Key: Shift left
        LeftCtrl = raylib::KEY_LEFT_CONTROL,       // Key: Control left
        LeftAlt = raylib::KEY_LEFT_ALT,            // Key: Alt left
        LeftSuper = raylib::KEY_LEFT_SUPER,        // Key: Super left
        RightShift = raylib::KEY_RIGHT_SHIFT,      // Key: Shift right
        RightCtrl = raylib::KEY_RIGHT_CONTROL,     // Key: Control right
        RightAlt = raylib::KEY_RIGHT_ALT,          // Key: Alt right
        RightSuper = raylib::KEY_RIGHT_SUPER,      // Key: Super right
        KB_Menu = raylib::KEY_KB_MENU,             // Key: KB menu
        NP_0 = raylib::KEY_KP_0,                   // Key: Keypad 0
        NP_1 = raylib::KEY_KP_1,                   // Key: Keypad 1
        NP_2 = raylib::KEY_KP_2,                   // Key: Keypad 2
        NP_3 = raylib::KEY_KP_3,                   // Key: Keypad 3
        NP_4 = raylib::KEY_KP_4,                   // Key: Keypad 4
        NP_5 = raylib::KEY_KP_5,                   // Key: Keypad 5
        NP_6 = raylib::KEY_KP_6,                   // Key: Keypad 6
        NP_7 = raylib::KEY_KP_7,                   // Key: Keypad 7
        NP_8 = raylib::KEY_KP_8,                   // Key: Keypad 8
        NP_9 = raylib::KEY_KP_9,                   // Key: Keypad 9
        NP_Decimal = raylib::KEY_KP_DECIMAL,       // Key: Keypad .
        NP_Divide = raylib::KEY_KP_DIVIDE,         // Key: Keypad /
        NP_MULTIPLY = raylib::KEY_KP_MULTIPLY,     // Key: Keypad *
        NP_Subtract = raylib::KEY_KP_SUBTRACT,     // Key: Keypad -
        NP_Add = raylib::KEY_KP_ADD,               // Key: Keypad +
        NP_Enter = raylib::KEY_KP_ENTER,           // Key: Keypad Enter
        NP_Equal = raylib::KEY_KP_EQUAL,           // Key: Keypad =
    };

    static constexpr inline std::array KeyboardButtonMap =
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

    enum class InputDevice
    {
        None,
        Unknown,
        Mouse,
        Keyboard,
        Gamepad
    };

    static constexpr inline std::array InputDeviceMap =
        std::to_array<std::pair<std::string_view, InputDevice>>({
            { "None", InputDevice::None },
            { "Mouse", InputDevice::Mouse },
            { "Keyboard", InputDevice::Keyboard },
            { "Gamepad", InputDevice::Gamepad },
        });

    enum class ActionCategory
    {
        None,
        UI,
        Game
    };

    static constexpr inline std::array ActionCategoryMap =
        std::to_array<std::pair<std::string_view, ActionCategory>>({
            { "None", ActionCategory::None },
            { "UI", ActionCategory::UI },
            { "Game", ActionCategory::Game },
        });

    enum class GameplayAction
    {
        None,
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
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

    static constexpr inline std::array GameActionMap =
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

    enum class UIAction
    {
        None,
        Up,
        Down,
        Left,
        Right,
        Next,
        Prev,
        Accept,
        Cancel,
    };

    static constexpr inline std::array UIActionMap =
        std::to_array<std::pair<std::string_view, UIAction>>({
            { "navigate_up", UIAction::Up },
            { "navigate_down", UIAction::Down },
            { "navigate_left", UIAction::Left },
            { "navigate_right", UIAction::Right },
            { "accept", UIAction::Accept },
            { "cancel", UIAction::Cancel },
        });

    template <typename TAction>
    struct ActionInput
    {
        std::string action_name;
        TAction action;
        Key button{ Key::Null };
        bool modifier_ctrl{ false };
        bool modifier_shift{ false };
        InputDevice device{ InputDevice::None };
        ActionCategory category{ ActionCategory::None };
    };

    struct Keymap
    {
        const static inline std::filesystem::path keymap_cfg{ "./data/configs/keymap.json" };

        Keymap()
        {
            simdjson::dom::parser config_parser{};
            simdjson::dom::element keymap{ config_parser.load(Keymap::keymap_cfg.string()) };
            m_game_input_mappings = load_action_inputs<GameplayAction>("game", keymap, GameActionMap);
            m_ui_input_mappings = load_action_inputs<UIAction>("ui", keymap, UIActionMap);
        }

        const std::vector<ActionInput<GameplayAction>>& game_action_keymap() const
        {
            return m_game_input_mappings;
        }

        const std::vector<ActionInput<UIAction>>& ui_action_keymap() const
        {
            return m_ui_input_mappings;
        }

    public:
        template <typename TAction>
        std::vector<ActionInput<TAction>> load_action_inputs(std::string category_name,
                                                             simdjson::dom::element& keymap,
                                                             auto text_to_keycode_map)
        {
            std::vector<ActionInput<TAction>> action_input_mappings{};
            action_input_mappings.reserve(128);

            auto game_actions = keymap[category_name];
            for (const auto& actions : text_to_keycode_map)
            {
                const auto& [action_label, action_type] = actions;
                auto&& curr_action_keys = game_actions[action_label];
                for (const auto&& action_keys : curr_action_keys)
                {
                    ActionInput<TAction> action = {
                        .action_name = std::string{ action_label },
                        .action = get_action<TAction>(std::string{ action_label }),
                        .button = get_button(std::string(action_keys["button"])),
                        .modifier_ctrl = false,
                        .modifier_shift = false,
                        .device = get_device(std::string(action_keys["device"])),
                        .category = ActionCategory::Game,
                    };

                    action_input_mappings.push_back(std::move(action));
                }
            }

            return action_input_mappings;
        }

        template <typename TAction>
        static TAction get_action(std::string name)
        {
            if constexpr (std::is_same_v<TAction, GameplayAction>)
            {
                for (const auto& mapping : GameActionMap)
                {
                    auto&& [action_name, action_type] = mapping;
                    if (name == action_name)
                        return action_type;
                }
                return GameplayAction::None;
            }
            if constexpr (std::is_same_v<TAction, UIAction>)
            {
                for (const auto& mapping : UIActionMap)
                {
                    auto&& [action_name, action_type] = mapping;
                    if (name == action_name)
                        return action_type;
                }
                return UIAction::None;
            }
        }

        template <typename TAction>
        static constexpr std::string get_action_name(TAction action)
        {
            if constexpr (std::is_same_v<TAction, GameplayAction>)
            {
                for (const auto& mapping : GameActionMap)
                {
                    auto&& [action_name, action_type] = mapping;
                    if (action_type == action)
                        return action_name.data();
                }
                return "none";
            }
            if constexpr (std::is_same_v<TAction, UIAction>)
            {
                for (const auto& mapping : UIActionMap)
                {
                    auto&& [action_name, action_type] = mapping;
                    if (action_type == action)
                        return action_name.data();
                }
                return "unknown";
            }
        }

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

    private:
        std::vector<ActionInput<UIAction>> m_ui_input_mappings{};
        std::vector<ActionInput<GameplayAction>> m_game_input_mappings{};
    };
}
