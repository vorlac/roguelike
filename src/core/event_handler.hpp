#pragma once

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/window.hpp"
#include "sdl/defs.hpp"
#include "utils/conversions.hpp"
#include "utils/io.hpp"
#include "utils/options.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl {
    class EventHandler
    {
    public:
        bool handle_events(std::unique_ptr<Window>& window)
        {
            SDL3::SDL_Event e{};

            while (SDL3::SDL_PollEvent(&e) != 0)
            {
                switch (e.type)
                {
                    // Mouse input events
                    case Mouse::Event::MouseWheel:
                        m_mouse.process_wheel(e.wheel);
                        window->on_mouse_scroll(e.wheel);
                        if constexpr (io::logging::mouse_events)
                            log::info("{}", m_mouse);
                        break;
                    case Mouse::Event::MouseMotion:
                        m_mouse.process_motion(e.motion);
                        if constexpr (io::logging::mouse_events)
                            log::info("{}", m_mouse);
                        break;
                    case Mouse::Event::MouseButtonDown:
                        m_mouse.process_button_down(e.button.button);
                        window->on_mouse_click(e.button.button);
                        if constexpr (io::logging::mouse_events)
                            log::info("{}", m_mouse);
                        break;
                    case Mouse::Event::MouseButtonUp:
                        m_mouse.process_button_up(e.button.button);
                        window->on_mouse_click(e.button.button);
                        if constexpr (io::logging::mouse_events)
                            log::info("{}", m_mouse);
                        break;

                    // Keyboard input events
                    case Keyboard::Event::KeyDown:
                        m_keyboard.process_button_down(e.key.keysym.scancode);
                        window->on_kb_key_pressed(e.key.keysym.scancode);
                        if constexpr (io::logging::kb_events)
                            log::info("{}", m_keyboard);
                        if (m_keyboard.is_button_pressed(Keyboard::Button::Escape)) [[unlikely]]
                            m_quit = true;
                        break;
                    case Keyboard::Event::KeyUp:
                        m_keyboard.process_button_up(e.key.keysym.scancode);
                        window->on_kb_key_pressed(e.key.keysym.scancode);
                        if constexpr (io::logging::kb_events)
                            log::info("{}", m_keyboard);
                        break;

                    // Window events
                    case Window::Event::Shown:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_shown(id);
                        break;
                    }
                    case Window::Event::Hidden:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_hidden(id);
                        break;
                    }
                    case Window::Event::Exposed:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_exposed(id);
                        break;
                    }
                    case Window::Event::Moved:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        const i32 x{ window_event.data1 };
                        const i32 y{ window_event.data2 };
                        window->on_moved(id, { x, y });
                        break;
                    }
                    case Window::Event::Resized:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        const i32 width{ window_event.data1 };
                        const i32 height{ window_event.data2 };
                        window->on_resized(id, { width, height });
                        break;
                    }
                    case Window::Event::PixelSizeChanged:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        const i32 width{ window_event.data1 };
                        const i32 height{ window_event.data2 };
                        window->on_pixel_size_changed(id, { width, height });
                        break;
                    }
                    case Window::Event::Minimized:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_minimized(id);
                        break;
                    }
                    case Window::Event::Maximized:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_maximized(id);
                        break;
                    }
                    case Window::Event::Restored:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_restored(id);
                        break;
                    }
                    case Window::Event::MouseEnter:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_mouse_enter(id);
                        break;
                    }
                    case Window::Event::MouseLeave:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_mouse_leave(id);
                        break;
                    }
                    case Window::Event::FocusGained:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_kb_focus_gained(id);
                        break;
                    }
                    case Window::Event::FocusLost:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_kb_focus_lost(id);
                        break;
                    }
                    case Window::Event::CloseRequested:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_close_requested(id);
                        break;
                    }
                    case Window::Event::TakeFocus:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_take_focus(id);
                        break;
                    }
                    case Window::Event::HitTest:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_hit_test(id);
                        break;
                    }
                    case Window::Event::ICCProfChanged:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_icc_profile_changed(id);
                        break;
                    }
                    case Window::Event::DisplayChanged:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_display_changed(id);
                        break;
                    }
                    case Window::Event::DisplayScaleChanged:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_display_scale_changed(id);
                        break;
                    }
                    case Window::Event::Occluded:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_occluded(id);
                        break;
                    }
                    case Window::Event::Destroyed:
                    {
                        const Window::Event::Data& window_event{ e.window };
                        const WindowID id{ window_event.windowID };
                        window->on_destroyed(id);
                        break;
                    }

                    // Display events
                    case Window::DisplayEvent::ContentScaleChanged:
                    {
                        const Window::DisplayEvent::Data& window_event{ e.display };
                        const DisplayID id{ window_event.displayID };
                        window->on_display_content_scale_changed(id);
                        break;
                    }

                    // Quit request event
                    case Event::Quit:
                        m_quit = true;
                        break;
                }
            }
            return true;
        }

        constexpr inline bool quit_triggered() const
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
