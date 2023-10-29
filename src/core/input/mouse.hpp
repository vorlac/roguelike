#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/utils/assert.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::input::device
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

        enum Button : i32 {
            Left    = raylib::MOUSE_BUTTON_LEFT,     // Left mouse button
            Right   = raylib::MOUSE_BUTTON_RIGHT,    // Right mouse button
            Middle  = raylib::MOUSE_BUTTON_MIDDLE,   // Middle mouse button (pressed wheel)
            Side    = raylib::MOUSE_BUTTON_SIDE,     // Side mouse button (advanced mouse device)
            Extra   = raylib::MOUSE_BUTTON_EXTRA,    // Extra mouse button (advanced mouse device)
            Forward = raylib::MOUSE_BUTTON_FORWARD,  // Forward button (advanced mouse device)
            Back    = raylib::MOUSE_BUTTON_BACK,     // Back button (advanced mouse device)
            Count
        };

        enum class Cursor : i32 {
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
            Count
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

        void set_x(int32_t x) const;
        void set_y(int32_t y) const;
        void set_position(int32_t x, int32_t y) const;
        void set_position(ds::point<int32_t> pos) const;
        void set_offset(int32_t x_offset = 0, int32_t y_offset = 0) const;
        void set_offset(ds::vector2<int32_t> offset) const;
        void set_scale(float x_scale = 1.0f, float y_scale = 1.0f) const;
        void set_scale(ds::vector2<float> scale) const;
        void set_cursor(CursorID cursor = raylib::MouseCursor::MOUSE_CURSOR_DEFAULT) const;

        int32_t get_x() const;
        int32_t get_y() const;

        ds::point<int32_t> get_position() const;
        ds::vector2<int32_t> get_delta() const;
        ds::vector2<float> get_wheel_move_v() const;
        float get_wheel_move() const;

        inline constexpr auto get_button_states(const bool check = true) const
        {
            constexpr size_t button_count{ std::to_underlying(Mouse::Button::Count) };
            for (size_t id = 0; check && id < button_count; ++id)
            {
                Mouse::ButtonState state = m_button_states[id];
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
                    std::forward<ds::vector2<int32_t>>(delta),
                };

                m_movement_state = std::make_pair(std::move(m_movement_state.second),
                                                  std::move(curr_state));
                return m_movement_state;
            }
        }

        static inline constexpr Mouse::ButtonID button_id(Mouse::Button button)
        {
            runtime_assert(button < Mouse::Button::Count, "invalid mouse button");
            return std::to_underlying(button);
        }

        static inline consteval Mouse::CursorID cursor_id(Mouse::Cursor cursor)
        {
            runtime_assert(cursor < Mouse::Cursor::Count, "invalid mouse cursor");
            return std::to_underlying(cursor);
        }

    private:
        // pair<prev state, curr state>
        mutable std::pair<std::pair<Mouse::CursorState, ds::vector2<int32_t>>,
                          std::pair<Mouse::CursorState, ds::vector2<int32_t>>>
            m_movement_state{
                { CursorState::None, ds::vector2<int32_t>::zero() },  // prev
                { CursorState::None, ds::vector2<int32_t>::zero() },  // curr
            };

        mutable std::array<ButtonState, static_cast<std::size_t>(Button::Count)> m_button_states{};
    };
}
