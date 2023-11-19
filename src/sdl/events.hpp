#pragma once

#include "sdl/defs.hpp"
#include "sdl/keyboard.hpp"
#include "sdl/mouse.hpp"
#include "utils/conversions.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl::sdl {
    class event_handler
    {
    public:
        bool handle_events()
        {
            SDL3::SDL_Event e{};

            constexpr bool log_events = false;
            while (SDL3::SDL_PollEvent(&e) != 0)
            {
                switch (e.type)
                {
                    // User requests quit
                    case Event::Quit:
                        m_quit = true;
                        break;
                    case Mouse::Event::MouseWheel:
                        m_mouse.process_wheel(e.wheel);
                        if constexpr (log_events)
                            log::info("{}", m_mouse);
                        break;
                    case Mouse::Event::MouseMotion:
                        m_mouse.process_motion(e.motion);
                        if constexpr (log_events)
                            log::info("{}", m_mouse);
                        break;
                    case Mouse::Event::MouseButtonDown:
                        m_mouse.process_button_down(e.button.button);
                        if constexpr (log_events)
                            log::info("{}", m_mouse);
                        break;
                    case Mouse::Event::MouseButtonUp:
                        m_mouse.process_button_up(e.button.button);
                        if constexpr (log_events)
                            log::info("{}", m_mouse);
                        break;
                    case Keyboard::Event::KeyDown:
                        m_keyboard.process_button_down(e.key.keysym.scancode);
                        if constexpr (log_events)
                            log::info("{}", m_keyboard);
                        break;
                    case Keyboard::Event::KeyUp:
                        m_keyboard.process_button_up(e.key.keysym.scancode);
                        if constexpr (log_events)
                            log::info("{}", m_keyboard);
                        break;
                }
            }
            return true;
        }

        inline bool quit_triggered() const
        {
            return m_quit;
        }

    private:
        bool m_quit = false;
        Mouse m_mouse{};
        Keyboard m_keyboard{};

    public:
        // clang-format off
        enum ButtonState {
            Pressed = 0,
            Released = 1,
        };

        struct EventAction {
            using type = SDL3::SDL_eventaction;
            static inline constexpr type Add = SDL3::SDL_ADDEVENT;
            static inline constexpr type Peek = SDL3::SDL_PEEKEVENT;
            static inline constexpr type Get = SDL3::SDL_GETEVENT;
        };

        struct Event {
            using type = SDL3::SDL_EventType;
            static inline constexpr type First = SDL3::SDL_EVENT_FIRST;
            static inline constexpr type Quit = SDL3::SDL_EVENT_QUIT;
            static inline constexpr type Terminating = SDL3::SDL_EVENT_TERMINATING;
            static inline constexpr type LowMemory = SDL3::SDL_EVENT_LOW_MEMORY;
            static inline constexpr type WillEnterBackground = SDL3::SDL_EVENT_WILL_ENTER_BACKGROUND;
            static inline constexpr type DidEnterBackground = SDL3::SDL_EVENT_DID_ENTER_BACKGROUND;
            static inline constexpr type WillEnterForeground = SDL3::SDL_EVENT_WILL_ENTER_FOREGROUND;
            static inline constexpr type DidEnterForeground = SDL3::SDL_EVENT_DID_ENTER_FOREGROUND;
            static inline constexpr type LocaleChanged = SDL3::SDL_EVENT_LOCALE_CHANGED;
            static inline constexpr type SystemThemeChanged = SDL3::SDL_EVENT_SYSTEM_THEME_CHANGED;

            static inline constexpr type DisplayOrientation = SDL3::SDL_EVENT_DISPLAY_ORIENTATION;
            static inline constexpr type DisplayAdded = SDL3::SDL_EVENT_DISPLAY_ADDED;
            static inline constexpr type DisplayRemoved = SDL3::SDL_EVENT_DISPLAY_REMOVED;
            static inline constexpr type DisplayMoved = SDL3::SDL_EVENT_DISPLAY_MOVED;
            static inline constexpr type DispContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED;
            static inline constexpr type DisplayFirst = SDL3::SDL_EVENT_DISPLAY_FIRST;
            static inline constexpr type DisplayLast = SDL3::SDL_EVENT_DISPLAY_LAST;

            static inline constexpr type WindowShown = SDL3::SDL_EVENT_WINDOW_SHOWN;
            static inline constexpr type WindowHidden = SDL3::SDL_EVENT_WINDOW_HIDDEN;
            static inline constexpr type WindowExposed = SDL3::SDL_EVENT_WINDOW_EXPOSED;
            static inline constexpr type WindowMoved = SDL3::SDL_EVENT_WINDOW_MOVED;
            static inline constexpr type WindowResized = SDL3::SDL_EVENT_WINDOW_RESIZED;
            static inline constexpr type WindowPixelSizeChanged = SDL3::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
            static inline constexpr type WindowFirst = SDL3::SDL_EVENT_WINDOW_FIRST;
            static inline constexpr type WindowMinimized = SDL3::SDL_EVENT_WINDOW_MINIMIZED;
            static inline constexpr type WindowMaximized = SDL3::SDL_EVENT_WINDOW_MAXIMIZED;
            static inline constexpr type WindowRestored = SDL3::SDL_EVENT_WINDOW_RESTORED;
            static inline constexpr type WindowMouseEnter = SDL3::SDL_EVENT_WINDOW_MOUSE_ENTER;
            static inline constexpr type WindowMouseLeave = SDL3::SDL_EVENT_WINDOW_MOUSE_LEAVE;
            static inline constexpr type WindowFocusGained = SDL3::SDL_EVENT_WINDOW_FOCUS_GAINED;
            static inline constexpr type WindowFocusLost = SDL3::SDL_EVENT_WINDOW_FOCUS_LOST;
            static inline constexpr type WindowCloseRequested = SDL3::SDL_EVENT_WINDOW_CLOSE_REQUESTED;
            static inline constexpr type WindowTakeFocus = SDL3::SDL_EVENT_WINDOW_TAKE_FOCUS;
            static inline constexpr type WindowHitTest = SDL3::SDL_EVENT_WINDOW_HIT_TEST;
            static inline constexpr type WindowIccprofChanged = SDL3::SDL_EVENT_WINDOW_ICCPROF_CHANGED;
            static inline constexpr type WindowDisplayChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_CHANGED;
            static inline constexpr type WindowDispScaleChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED;
            static inline constexpr type WindowOccluded = SDL3::SDL_EVENT_WINDOW_OCCLUDED;
            static inline constexpr type WindowDestroyed = SDL3::SDL_EVENT_WINDOW_DESTROYED;
            static inline constexpr type WindowLast = SDL3::SDL_EVENT_WINDOW_LAST;

            static inline constexpr type KeyDown = SDL3::SDL_EVENT_KEY_DOWN;
            static inline constexpr type KeyUp = SDL3::SDL_EVENT_KEY_UP;
            static inline constexpr type TextEditing = SDL3::SDL_EVENT_TEXT_EDITING;
            static inline constexpr type TextInput = SDL3::SDL_EVENT_TEXT_INPUT;
            static inline constexpr type KeymapChanged = SDL3::SDL_EVENT_KEYMAP_CHANGED;

            static inline constexpr type MouseMotion = SDL3::SDL_EVENT_MOUSE_MOTION;
            static inline constexpr type MouseButtonDown = SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN;
            static inline constexpr type MouseButtonUp = SDL3::SDL_EVENT_MOUSE_BUTTON_UP;
            static inline constexpr type MouseWheel = SDL3::SDL_EVENT_MOUSE_WHEEL;

            static inline constexpr type JoystickAxisMotion = SDL3::SDL_EVENT_JOYSTICK_AXIS_MOTION;
            static inline constexpr type JoystickHatMotion = SDL3::SDL_EVENT_JOYSTICK_HAT_MOTION;
            static inline constexpr type JoystickButtonDown = SDL3::SDL_EVENT_JOYSTICK_BUTTON_DOWN;
            static inline constexpr type JoystickButtonUp = SDL3::SDL_EVENT_JOYSTICK_BUTTON_UP;
            static inline constexpr type JoystickAdded = SDL3::SDL_EVENT_JOYSTICK_ADDED;
            static inline constexpr type JoystickRemoved = SDL3::SDL_EVENT_JOYSTICK_REMOVED;
            static inline constexpr type JoystickBatteryUpdated = SDL3::SDL_EVENT_JOYSTICK_BATTERY_UPDATED;
            static inline constexpr type JoystickUpdateComplete = SDL3::SDL_EVENT_JOYSTICK_UPDATE_COMPLETE;

            static inline constexpr type GamepadAxisMotion = SDL3::SDL_EVENT_GAMEPAD_AXIS_MOTION;
            static inline constexpr type GamepadButtonDown = SDL3::SDL_EVENT_GAMEPAD_BUTTON_DOWN;
            static inline constexpr type GamepadButtonUp = SDL3::SDL_EVENT_GAMEPAD_BUTTON_UP;
            static inline constexpr type GamepadAdded = SDL3::SDL_EVENT_GAMEPAD_ADDED;
            static inline constexpr type GamepadRemoved = SDL3::SDL_EVENT_GAMEPAD_REMOVED;
            static inline constexpr type GamepadRemapped = SDL3::SDL_EVENT_GAMEPAD_REMAPPED;
            static inline constexpr type GamepadTouchpadDown = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN;
            static inline constexpr type GamepadTouchpadMotion = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION;
            static inline constexpr type GamepadTouchpadUp = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_UP;
            static inline constexpr type GamepadSensorUpdate = SDL3::SDL_EVENT_GAMEPAD_SENSOR_UPDATE;
            static inline constexpr type GamepadUpdateComplete = SDL3::SDL_EVENT_GAMEPAD_UPDATE_COMPLETE;

            static inline constexpr type FingerDown = SDL3::SDL_EVENT_FINGER_DOWN;
            static inline constexpr type FingerUp = SDL3::SDL_EVENT_FINGER_UP;
            static inline constexpr type FingerMotion = SDL3::SDL_EVENT_FINGER_MOTION;

            static inline constexpr type ClipboardUpdate = SDL3::SDL_EVENT_CLIPBOARD_UPDATE;
            static inline constexpr type DropFile = SDL3::SDL_EVENT_DROP_FILE;
            static inline constexpr type DropText = SDL3::SDL_EVENT_DROP_TEXT;
            static inline constexpr type DropBegin = SDL3::SDL_EVENT_DROP_BEGIN;
            static inline constexpr type DropComplete = SDL3::SDL_EVENT_DROP_COMPLETE;
            static inline constexpr type DropPosition = SDL3::SDL_EVENT_DROP_POSITION;

            static inline constexpr type AudioDeviceAdded = SDL3::SDL_EVENT_AUDIO_DEVICE_ADDED;
            static inline constexpr type AudioDeviceRemoved = SDL3::SDL_EVENT_AUDIO_DEVICE_REMOVED;
            static inline constexpr type AudioDeviceFormatChanged = SDL3::SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED;

            static inline constexpr type SensorUpdate = SDL3::SDL_EVENT_SENSOR_UPDATE;
            static inline constexpr type RenderTargetsReset = SDL3::SDL_EVENT_RENDER_TARGETS_RESET;
            static inline constexpr type RenderDeviceReset = SDL3::SDL_EVENT_RENDER_DEVICE_RESET;
            static inline constexpr type PollSentinel = SDL3::SDL_EVENT_POLL_SENTINEL;
            static inline constexpr type User = SDL3::SDL_EVENT_USER;

            static inline constexpr type Last = SDL3::SDL_EVENT_LAST;
        };

        // clang-format on
    };
}
