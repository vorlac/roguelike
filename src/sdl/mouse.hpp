#pragma once

#include <fmt/format.h>

#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"

namespace SDL3 {
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
}

namespace rl::sdl {
    class Mouse
    {
    public:
        using MotionEvent = SDL3::SDL_MouseMotionEvent;
        using WheelEvent = SDL3::SDL_MouseWheelEvent;

        struct Event
        {
            using type = SDL3::SDL_EventType;
            constexpr static inline type MouseMotion = SDL3::SDL_EVENT_MOUSE_MOTION;
            constexpr static inline type MouseButtonDown = SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN;
            constexpr static inline type MouseButtonUp = SDL3::SDL_EVENT_MOUSE_BUTTON_UP;
            constexpr static inline type MouseWheel = SDL3::SDL_EVENT_MOUSE_WHEEL;
        };

        struct Button
        {
            using type = rl::u8;
            constexpr static inline type Left = SDL_BUTTON_LEFT;
            constexpr static inline type Middle = SDL_BUTTON_MIDDLE;
            constexpr static inline type Right = SDL_BUTTON_RIGHT;
            constexpr static inline type X1 = SDL_BUTTON_X1;
            constexpr static inline type X2 = SDL_BUTTON_X2;
            constexpr static inline type Count = Button::X2;
        };

        enum ButtonMask : rl::u8 {
            Left = SDL_BUTTON_LMASK,
            Middle = SDL_BUTTON_MMASK,
            Right = SDL_BUTTON_RMASK,
            X1 = SDL_BUTTON_X1MASK,
            X2 = SDL_BUTTON_X2MASK,
        };

        struct Cursor
        {
            using type = SDL3::SDL_SystemCursor;
            constexpr static inline type Arrow = SDL3::SDL_SYSTEM_CURSOR_ARROW;
            constexpr static inline type IBeam = SDL3::SDL_SYSTEM_CURSOR_IBEAM;
            constexpr static inline type Wait = SDL3::SDL_SYSTEM_CURSOR_WAIT;
            constexpr static inline type Crosshair = SDL3::SDL_SYSTEM_CURSOR_CROSSHAIR;
            constexpr static inline type WaitArrow = SDL3::SDL_SYSTEM_CURSOR_WAITARROW;
            constexpr static inline type SizeNWSE = SDL3::SDL_SYSTEM_CURSOR_SIZENWSE;
            constexpr static inline type SizeNESW = SDL3::SDL_SYSTEM_CURSOR_SIZENESW;
            constexpr static inline type SizeWE = SDL3::SDL_SYSTEM_CURSOR_SIZEWE;
            constexpr static inline type SizeNS = SDL3::SDL_SYSTEM_CURSOR_SIZENS;
            constexpr static inline type SizeAll = SDL3::SDL_SYSTEM_CURSOR_SIZEALL;
            constexpr static inline type No = SDL3::SDL_SYSTEM_CURSOR_NO;
            constexpr static inline type Hand = SDL3::SDL_SYSTEM_CURSOR_HAND;
            constexpr static inline type CursorCount = SDL3::SDL_NUM_SYSTEM_CURSORS;
        };

        struct WheelDirection
        {
            using type = rl::u8;
            constexpr static inline type Normal = SDL3::SDL_MOUSEWHEEL_NORMAL;
            constexpr static inline type Flipped = SDL3::SDL_MOUSEWHEEL_FLIPPED;
        };

    public:
        void process_button_down(Mouse::Button::type mouse_button)
        {
            runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");
            switch (mouse_button)
            {
                case Mouse::Button::Left:
                    [[fallthrough]];
                case Mouse::Button::Middle:
                    [[fallthrough]];
                case Mouse::Button::Right:
                    [[fallthrough]];
                case Mouse::Button::X1:
                    [[fallthrough]];
                case Mouse::Button::X2:
                    m_button_states |= (1 << (mouse_button - 1));
                    break;
            }
        }

        void process_button_up(Mouse::Button::type mouse_button)
        {
            runtime_assert(mouse_button - 1 < Mouse::Button::Count, "invalid mouse button");
            m_button_states &= ~(1 << (mouse_button - 1));
        }

        void process_motion(Mouse::MotionEvent& motion)
        {
            m_prev_cursor_pos = m_cursor_position;
            m_cursor_position = { motion.x, motion.y };
        }

        void process_wheel(Mouse::WheelEvent& wheel)
        {
            m_prev_wheel_pos = m_wheel_position;
            if (wheel.direction == Mouse::WheelDirection::Flipped)
            {
                wheel.x *= -1;
                wheel.y *= -1;
            }

            if (wheel.x != 0.0f)
            {
                // positive to the right and negative to the left
                m_wheel_position.x += wheel.x * 10.0f;
            }

            if (wheel.x != 0.0f)
            {
                // positive away from the user and negative towards the user
                m_wheel_position.y -= wheel.x * 10.0f;
            }
        }

    public:
        const ds::point<f32>& pos() const
        {
            return m_cursor_position;
        }

        const ds::vector2<f32>& wheel() const
        {
            return m_wheel_position;
        }

        ds::point<f32> pos_delta() const
        {
            return m_prev_cursor_pos - m_cursor_position;
        }

        bool is_button_down(Mouse::Button::type button) const
        {
            return 0 != (m_button_states & SDL_BUTTON(button));
        }

    private:
        rl::u32 m_button_states{ 0 };
        ds::point<f32> m_cursor_position{ 0, 0 };
        ds::point<f32> m_prev_cursor_pos{ 0, 0 };
        ds::vector2<f32> m_wheel_position{ 0.0f, 0.0f };
        ds::vector2<f32> m_prev_wheel_pos{ 0.0f, 0.0f };
    };
}

namespace rl::sdl {
    inline auto format_as(const Mouse& mouse)
    {
        return fmt::format("Mouse[pos={} l={}, m={}, r={}, wheel={}]", mouse.pos(),
                           mouse.is_button_down(Mouse::Button::Left),
                           mouse.is_button_down(Mouse::Button::Middle),
                           mouse.is_button_down(Mouse::Button::Right), mouse.wheel());
    }
}