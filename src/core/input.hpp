// #pragma once
//
// #include <fmt/format.h>
//
// #include "core/input/gamepad.hpp"
// #include "core/input/keyboard.hpp"
// #include "core/input/keymap.hpp"
// #include "core/input/mouse.hpp"
// #include "core/math.hpp"
// #include "core/numeric_types.hpp"
// #include "utils/assert.hpp"
//
// namespace rl {
//     struct ButtonInfo
//     {
//         u32 id{ 0 };
//         std::string_view description{};
//     };
//
//     struct ButtonState
//     {
//         bool pressed{ false };
//         bool released{ false };
//         bool held{ false };
//         i32 axis{ 9999 };
//     };
//
//     struct InputEvent
//     {
//         input::InputDevice device{ input::InputDevice::None };
//         ButtonInfo button{};
//     };
//
//     class Input
//     {
//     public:
//         Input()
//         {
//             m_active_game_actions.reserve(64);
//             m_active_ui_actions.reserve(64);
//         }
//
//         std::vector<input::GameplayAction> active_game_actions()
//         {
//             m_active_game_actions.clear();
//
//             const auto& game_actions = m_keymap.game_action_keymap();
//             for (const auto& action : game_actions)
//             {
//                 auto button = std::to_underlying(action.button);
//                 switch (action.device)
//                 {
//                     case input::InputDevice::None:
//                         [[fallthrough]];
//                     case input::InputDevice::Unknown:
//                         assert_msg("invalid input type");
//                         break;
//
//                     case input::InputDevice::Mouse:
//                         [[fallthrough]];
//                     case input::InputDevice::Gamepad:
//                         break;
//
//                     case input::InputDevice::Keyboard:
//                         if (keyboard.is_key_down(button))
//                             m_active_game_actions.push_back(action.action);
//                 }
//             }
//
//             return m_active_game_actions;
//         }
//
//         std::vector<input::UIAction> active_ui_actions()
//         {
//             m_active_ui_actions.clear();
//
//             const auto& game_actions = m_keymap.ui_action_keymap();
//             for (const auto& action : game_actions)
//             {
//                 auto button = std::to_underlying(action.button);
//                 switch (action.device)
//                 {
//                     case input::InputDevice::None:
//                         [[fallthrough]];
//                     case input::InputDevice::Unknown:
//                         assert_msg("invalid input type");
//                         break;
//
//                     case input::InputDevice::Mouse:
//                         [[fallthrough]];
//                     case input::InputDevice::Gamepad:
//                         break;
//
//                     case input::InputDevice::Keyboard:
//                     {
//                         if (keyboard.is_key_down(button))
//                             m_active_ui_actions.push_back(action.action);
//                         else
//                             this->set_selection(false);
//                         break;
//                     }
//                 }
//             }
//
//             return m_active_ui_actions;
//         }
//
//         constexpr inline auto mouse_button_states() const
//         {
//             return mouse.get_button_states();
//         }
//
//         constexpr inline auto mouse_cursor_states() const
//         {
//             return mouse.get_cursor_states();
//         }
//
//         inline ds::point<i32> mouse_cursor_position() const
//         {
//             return mouse.get_position();
//         }
//
//         inline ds::vector2<i32> mouse_cursor_delta() const
//         {
//             return mouse.get_delta();
//         }
//
//         inline void set_selection(const bool picked, u64 id = 0)
//         {
//             m_selection.exchange(std::make_pair(picked, id), std::memory_order_relaxed);
//         }
//
//         inline std::pair<bool, u64> get_selection() const
//         {
//             return m_selection.load(std::memory_order_relaxed);
//         }
//
//     private:
//         ds::vector2<f32> get_vector() const
//         {
//             input::Gamepad::AxisID pos_x{ 0 };
//             input::Gamepad::AxisID neg_x{ 1 };
//             input::Gamepad::AxisID pos_y{ 2 };
//             input::Gamepad::AxisID neg_y{ 3 };
//
//             ds::vector2<f32> vec = {
//                 gamepad.get_axis_movement(pos_x) - gamepad.get_axis_movement(neg_x),
//                 gamepad.get_axis_movement(pos_y) - gamepad.get_axis_movement(neg_y),
//             };
//
//             const f32 deadzone{ 0.1f };
//             const f32 length{ vec.length() };
//             if (length <= deadzone)
//                 return { 0.0f, 0.0f };
//             else if (length > 1.0f)
//                 return { vec / length };
//             else
//                 return vec * (rl::math::inverse_lerp(deadzone, 1.0f, length) / length);
//         }
//
//     public:
//         input::Mouse mouse{};
//         input::Keyboard keyboard{};
//         input::Gamepad gamepad{};
//
//     private:
//         std::atomic<std::pair<bool, u64>> m_selection{ std::make_pair(false, 0) };
//
//         input::Keymap m_keymap{};
//         std::vector<input::GameplayAction> m_active_game_actions{};
//         std::vector<input::UIAction> m_active_ui_actions{};
//     };
// }
