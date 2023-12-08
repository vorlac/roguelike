#pragma once

#include <type_traits>

#include <fmt/format.h>

#include "ds/point.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
SDL_C_LIB_END

namespace rl::sdl {
    class Mouse
    {
    public:
        struct Event
        {
            using type_t = SDL3::SDL_EventType;
            using type = std::underlying_type_t<type_t>;

            enum ID : type {
                MouseMotion = SDL3::SDL_EVENT_MOUSE_MOTION,
                MouseButtonDown = SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN,
                MouseButtonUp = SDL3::SDL_EVENT_MOUSE_BUTTON_UP,
                MouseWheel = SDL3::SDL_EVENT_MOUSE_WHEEL,
            };

            struct Data
            {
                using Motion = SDL3::SDL_MouseMotionEvent;
                using Wheel = SDL3::SDL_MouseWheelEvent;
            };
        };

        struct Button
        {
            using type = u8;

            enum ID : type {
                Left = SDL_BUTTON_LEFT,
                Middle = SDL_BUTTON_MIDDLE,
                Right = SDL_BUTTON_RIGHT,
                X1 = SDL_BUTTON_X1,
                X2 = SDL_BUTTON_X2,
                Count = Button::X2,
            };
        };

        struct Cursor
        {
            using type = SDL3::SDL_SystemCursor;

            enum ID : std::underlying_type_t<type> {
                Arrow = SDL3::SDL_SYSTEM_CURSOR_ARROW,
                IBeam = SDL3::SDL_SYSTEM_CURSOR_IBEAM,
                Wait = SDL3::SDL_SYSTEM_CURSOR_WAIT,
                Crosshair = SDL3::SDL_SYSTEM_CURSOR_CROSSHAIR,
                WaitArrow = SDL3::SDL_SYSTEM_CURSOR_WAITARROW,
                SizeNWSE = SDL3::SDL_SYSTEM_CURSOR_SIZENWSE,
                SizeNESW = SDL3::SDL_SYSTEM_CURSOR_SIZENESW,
                SizeWE = SDL3::SDL_SYSTEM_CURSOR_SIZEWE,
                SizeNS = SDL3::SDL_SYSTEM_CURSOR_SIZENS,
                SizeAll = SDL3::SDL_SYSTEM_CURSOR_SIZEALL,
                No = SDL3::SDL_SYSTEM_CURSOR_NO,
                Hand = SDL3::SDL_SYSTEM_CURSOR_HAND,
                CursorCount = SDL3::SDL_NUM_SYSTEM_CURSORS,
            };
        };

        struct Wheel
        {
            using type = u8;

            enum Direction : type {
                Normal = SDL3::SDL_MOUSEWHEEL_NORMAL,
                Flipped = SDL3::SDL_MOUSEWHEEL_FLIPPED,
            };
        };

    public:
        void process_button_down(Mouse::Button::type mouse_button)
        {
            runtime_assert((mouse_button - 1) < Mouse::Button::Count, "invalid mouse button");
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

        void process_motion(Event::Data::Motion& motion)
        {
            m_prev_cursor_pos = m_cursor_position;
            m_cursor_position = { motion.x, motion.y };
        }

        void process_wheel(Mouse::Event::Data::Wheel& wheel)
        {
            m_prev_wheel_pos = m_wheel_position;
            if (wheel.direction == Mouse::Wheel::Direction::Flipped)
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

        bool is_button_down(u32 button) const
        {
            return 0 != (m_button_states & SDL_BUTTON(button));
        }

    private:
        rl::u32 m_button_states{ 0 };
        ds::point<f32> m_cursor_position{ 0.0f, 0.0f };
        ds::point<f32> m_prev_cursor_pos{ 0.0f, 0.0f };
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
