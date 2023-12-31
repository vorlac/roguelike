#pragma once

#include "ds/point.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
SDL_C_LIB_END

namespace rl {
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
                WindowTopLeft = SDL3::SDL_SYSTEM_CURSOR_WINDOW_TOPLEFT,
                WindowTop = SDL3::SDL_SYSTEM_CURSOR_WINDOW_TOP,
                WindowTopRight = SDL3::SDL_SYSTEM_CURSOR_WINDOW_TOPRIGHT,
                WindowRight = SDL3::SDL_SYSTEM_CURSOR_WINDOW_RIGHT,
                WindowBotRight = SDL3::SDL_SYSTEM_CURSOR_WINDOW_BOTTOMRIGHT,
                WindowBottom = SDL3::SDL_SYSTEM_CURSOR_WINDOW_BOTTOM,
                WindowBotLeft = SDL3::SDL_SYSTEM_CURSOR_WINDOW_BOTTOMLEFT,
                WindowLeft = SDL3::SDL_SYSTEM_CURSOR_WINDOW_LEFT,
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

    protected:
        friend class EventHandler;
        void process_button_down(Mouse::Button::type mouse_button);
        void process_button_up(Mouse::Button::type mouse_button);
        void process_motion(Event::Data::Motion& motion);
        void process_wheel(Mouse::Event::Data::Wheel& wheel);

    public:
        ds::point<i32> pos() const;
        ds::vector2<i32> wheel() const;
        ds::vector2<i32> pos_delta() const;
        bool is_button_down(u32 button) const;

    private:
        rl::u32 m_button_states{ 0 };
        ds::point<i32> m_cursor_position{ 0, 0 };
        ds::point<i32> m_prev_cursor_pos{ 0, 0 };
        ds::vector2<i32> m_wheel_position{ 0, 0 };
        ds::vector2<i32> m_prev_wheel_pos{ 0, 0 };
    };
}

namespace rl {
    inline auto format_as(const Mouse& mouse)
    {
        return fmt::format("Mouse[pos={} l={}, m={}, r={}, wheel={}]", mouse.pos(),
                           mouse.is_button_down(Mouse::Button::Left),
                           mouse.is_button_down(Mouse::Button::Middle),
                           mouse.is_button_down(Mouse::Button::Right), mouse.wheel());
    }
}
