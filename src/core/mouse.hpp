#pragma once

#include "ds/point.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
SDL_C_LIB_END

namespace rl {
    class Mouse
    {
    public:
        friend class MainWindow;

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

    public:
        [[nodiscard]] ds::point<i32> pos() const;
        [[nodiscard]] ds::vector2<i32> wheel() const;
        [[nodiscard]] ds::vector2<i32> pos_delta() const;
        [[nodiscard]] Mouse::Button::ID button_pressed() const;
        [[nodiscard]] Mouse::Button::ID button_released() const;

        [[nodiscard]] bool is_button_pressed(Mouse::Button::ID button) const;
        [[nodiscard]] bool is_button_released(Mouse::Button::ID button) const;
        [[nodiscard]] bool is_button_held(Mouse::Button::ID button) const;
        [[nodiscard]] bool is_button_down(Mouse::Button::ID button) const;
        [[nodiscard]] bool all_buttons_down(std::vector<Mouse::Button::ID> buttons) const;
        [[nodiscard]] bool any_buttons_down(std::vector<Mouse::Button::ID> buttons) const;

        [[nodiscard]] std::string get_button_state(Mouse::Button::ID button) const;

    protected:
        void process_button_down(Mouse::Button::ID mouse_button);
        void process_button_up(Mouse::Button::ID mouse_button);
        void process_motion(const Event::Data::Motion& motion);
        void process_wheel(const Mouse::Event::Data::Wheel& wheel);

    private:
        u32 m_button_states{ 0 };
        u32 m_buttons_held{ 0 };
        u32 m_buttons_pressed{ 0 };
        u32 m_buttons_released{ 0 };
        ds::point<i32> m_cursor_position{ 0, 0 };
        ds::point<i32> m_prev_cursor_pos{ 0, 0 };
        ds::vector2<i32> m_wheel_position{ 0, 0 };
        ds::vector2<i32> m_prev_wheel_pos{ 0, 0 };
    };
}

namespace rl {
    inline auto format_as(const Mouse& mouse)
    {
        return fmt::format("pos={} lmb={}, rmb={}, wheel=[{} | {}]", mouse.pos(),
                           mouse.get_button_state(Mouse::Button::Left),
                           mouse.get_button_state(Mouse::Button::Right),
                           mouse.get_button_state(Mouse::Button::Middle), mouse.wheel());
    }
}
