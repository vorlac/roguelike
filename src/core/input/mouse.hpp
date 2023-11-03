#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::input
{
    class Mouse
    {
    public:
        enum class ButtonState {
            None,
            Pressed,
            Held,
            Released,
        };

        enum class CursorState {
            None,
            Moving,
            Still,
            Disabled,
        };

        enum Button : i16_fast {
            Left    = raylib::MOUSE_BUTTON_LEFT,     // Left mouse button
            Right   = raylib::MOUSE_BUTTON_RIGHT,    // Right mouse button
            Middle  = raylib::MOUSE_BUTTON_MIDDLE,   // Middle mouse button (pressed wheel)
            Side    = raylib::MOUSE_BUTTON_SIDE,     // Side mouse button (advanced mouse device)
            Extra   = raylib::MOUSE_BUTTON_EXTRA,    // Extra mouse button (advanced mouse device)
            Forward = raylib::MOUSE_BUTTON_FORWARD,  // Forward button (advanced mouse device)
            Back    = raylib::MOUSE_BUTTON_BACK,     // Back button (advanced mouse device)
            MouseButtonCount
        };

        enum Cursor : i16_fast {
            Default      = raylib::MOUSE_CURSOR_DEFAULT,
            Arrow        = raylib::MOUSE_CURSOR_ARROW,
            IBeam        = raylib::MOUSE_CURSOR_IBEAM,
            Cross        = raylib::MOUSE_CURSOR_CROSSHAIR,
            Hand         = raylib::MOUSE_CURSOR_POINTING_HAND,
            HorizResize  = raylib::MOUSE_CURSOR_RESIZE_EW,
            VertResize   = raylib::MOUSE_CURSOR_RESIZE_NS,
            TLtoBRResize = raylib::MOUSE_CURSOR_RESIZE_NWSE,
            TRtoBLResize = raylib::MOUSE_CURSOR_RESIZE_NESW,
            OmniResize   = raylib::MOUSE_CURSOR_RESIZE_ALL,
            Disabled     = raylib::MOUSE_CURSOR_NOT_ALLOWED,
            MouseCursorCount
        };

        using ButtonID = std::underlying_type_t<Mouse::Button>;
        using CursorID = std::underlying_type_t<Mouse::Cursor>;

    public:
        // checks if mouse button was pressed once
        template <typename TButton = Mouse::ButtonID>
        inline bool is_button_pressed(TButton button) const
        {
            auto button_id{ static_cast<Mouse::ButtonID>(button) };
            return raylib::IsMouseButtonPressed(button_id);
        }

        // checks if mouse button is being pressed
        template <typename TButton = Mouse::ButtonID>
        inline bool is_button_down(TButton button) const
        {
            auto button_id{ static_cast<Mouse::ButtonID>(button) };
            return raylib::IsMouseButtonDown(button_id);
        }

        // checks if mouse button was released once
        template <typename TButton = Mouse::ButtonID>
        inline bool is_button_released(TButton button) const
        {
            auto button_id{ static_cast<Mouse::ButtonID>(button) };
            return raylib::IsMouseButtonReleased(button_id);
        }

        // checks if mouse button is not pressed
        template <typename TButton = Mouse::ButtonID>
        inline bool is_button_up(TButton button) const
        {
            auto button_id{ static_cast<Mouse::ButtonID>(button) };
            return raylib::IsMouseButtonUp(button_id);
        }

        void set_x(i32 x) const;
        void set_y(i32 y) const;
        void set_offset(i32 x_offset = 0, i32 y_offset = 0) const;
        void set_offset(ds::vector2<i32> offset) const;
        void set_scale(float x_scale = 1.0f, float y_scale = 1.0f) const;
        void set_scale(ds::vector2<float> scale) const;
        void set_cursor(CursorID cursor = raylib::MouseCursor::MOUSE_CURSOR_DEFAULT) const;
        void hide_cursor() const;
        void show_cursor() const;

        i32 get_x() const;
        i32 get_y() const;

        template <rl::numeric T>
        void set_position(T x, T y) const
        {
            return raylib::SetMousePosition(cast::to<i32>(x), cast::to<i32>(y));
        }

        template <rl::numeric T>
        void set_position(ds::point<i32> pos) const
        {
            return raylib::SetMousePosition(cast::to<i32>(pos.x), cast::to<i32>(pos.y));
        }

        ds::point<i32> get_position() const;
        ds::vector2<i32> get_delta() const;
        ds::vector2<float> get_wheel_move_v() const;
        float get_wheel_move() const;

        inline constexpr auto get_button_states(const bool check = true) const
        {
            constexpr ButtonID button_count{ Mouse::Button::MouseButtonCount };
            for (ButtonID id = 0; check && id < button_count; ++id)
            {
                Mouse::ButtonState& state = m_button_states[cast::to<u32>(id)];
                state = this->is_button_down(id) ? ((state != Mouse::ButtonState::Held &&  //
                                                     state != ButtonState::Pressed)
                                                        ? ButtonState::Pressed
                                                        : ButtonState::Held)
                                                 : ((state == Mouse::ButtonState::Pressed ||  //
                                                     state == Mouse::ButtonState::Held)
                                                        ? ButtonState::Released
                                                        : ButtonState::None);
            }

            return m_button_states;
        }

        inline constexpr auto get_cursor_states(const bool check = true) const
        {
            if (!check)
                return m_movement_state;
            else
            {
                auto&& delta = this->get_delta();

                std::pair curr_state = {
                    delta.is_zero() ? CursorState::Moving : CursorState::Still,
                    std::forward<ds::vector2<i32>>(delta),
                };

                m_movement_state = std::make_pair(std::move(m_movement_state.second),
                                                  std::move(curr_state));
                return m_movement_state;
            }
        }

        static inline constexpr Mouse::ButtonID button_id(Mouse::Button button)
        {
            assertion(button < Mouse::Button::MouseButtonCount, "invalid mouse button");
            return std::to_underlying(button);
        }

        static inline consteval Mouse::CursorID cursor_id(Mouse::Cursor cursor)
        {
            assertion(cursor < Mouse::Cursor::MouseCursorCount, "invalid mouse cursor");
            return std::to_underlying(cursor);
        }

    private:
        // pair<prev state, curr state>
        mutable std::pair<std::pair<Mouse::CursorState, ds::vector2<i32>>,
                          std::pair<Mouse::CursorState, ds::vector2<i32>>>
            m_movement_state{
                { CursorState::None, ds::vector2<i32>::zero() },  // prev
                { CursorState::None, ds::vector2<i32>::zero() },  // curr
            };

        mutable std::array<ButtonState, Button::MouseButtonCount> m_button_states{};
    };
}
