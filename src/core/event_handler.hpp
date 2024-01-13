#pragma once

#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "core/system.hpp"
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
        bool handle_events(std::unique_ptr<MainWindow>& window)
        {
            SDL3::SDL_Event e{};

            while (SDL3::SDL_PollEvent(&e) != 0)
            {
                switch (e.type)
                {
                    // Mouse input events
                    case Mouse::Event::MouseWheel:
                        window->mouse_wheel_event_callback(e);
                        break;
                    case Mouse::Event::MouseMotion:
                        window->mouse_moved_event_callback(e);
                        break;
                    case Mouse::Event::MouseButtonDown:
                        window->mouse_button_pressed_event_callback(e);
                        break;
                    case Mouse::Event::MouseButtonUp:
                        window->mouse_button_released_event_callback(e);
                        break;

                    // Keyboard input events
                    case Keyboard::Event::KeyDown:
                    {
                        window->keyboard_key_pressed_event_callback(e);
                        Keyboard::Scancode::ID key{ e.key.keysym.scancode };
                        if (key == Keyboard::Scancode::Escape) [[unlikely]]
                            m_quit = true;
                        break;
                    }
                    case Keyboard::Event::KeyUp:
                        window->keyboard_key_released_event_callback(e);
                        break;
                    case Keyboard::Event::TextEditing:
                        window->keyboard_char_event_callback(e);
                        break;
                    case Keyboard::Event::TextInput:
                        window->keyboard_char_event_callback(e);
                        break;

                    // MainWindow events
                    case MainWindow::Event::Shown:
                        window->window_shown_event_callback(e);
                        break;
                    case MainWindow::Event::Hidden:
                        window->window_hidden_event_callback(e);
                        break;
                    case MainWindow::Event::Exposed:
                        window->window_exposed_event_callback(e);
                        break;
                    case MainWindow::Event::Moved:
                        window->window_moved_event_callback(e);
                        break;
                    case MainWindow::Event::Resized:
                        window->window_resized_event_callback(e);
                        break;
                    case MainWindow::Event::PixelSizeChanged:
                        window->window_pixel_size_changed_event_callback(e);
                        break;
                    case MainWindow::Event::Minimized:
                        window->window_minimized_event_callback(e);
                        break;
                    case MainWindow::Event::Maximized:
                        window->window_maximized_event_callback(e);
                        break;
                    case MainWindow::Event::Restored:
                        window->window_restored_event_callback(e);
                        break;
                    case MainWindow::Event::MouseEnter:
                        window->mouse_entered_event_callback(e);
                        break;
                    case MainWindow::Event::MouseLeave:
                        window->mouse_exited_event_callback(e);
                        break;
                    case MainWindow::Event::FocusGained:
                        window->window_focus_gained_event_callback(e);
                        break;
                    case MainWindow::Event::FocusLost:
                        window->window_focus_lost_event_callback(e);
                        break;
                    case MainWindow::Event::CloseRequested:
                        window->window_close_requested_event_callback(e);
                        break;
                    case MainWindow::Event::TakeFocus:
                        window->window_take_focus_event_callback(e);
                        break;
                    case MainWindow::Event::HitTest:
                        window->window_hit_test_event_callback(e);
                        break;
                    case MainWindow::Event::ICCProfChanged:
                        window->window_icc_profile_changed_callback(e);
                        break;
                    case MainWindow::Event::DisplayChanged:
                        window->window_display_changed_event_callback(e);
                        break;
                    case MainWindow::Event::DisplayScaleChanged:
                        window->window_display_scale_changed_event_callback(e);
                        break;
                    case MainWindow::Event::Occluded:
                        window->window_occluded_event_callback(e);
                        break;
                    case MainWindow::Event::Destroyed:
                        window->window_destroyed_event_callback(e);
                        break;

                    // Display events
                    case MainWindow::DisplayEvent::ContentScaleChanged:
                    {
                        const MainWindow::DisplayEvent::Data& window_event{ e.display };
                        const DisplayID id{ window_event.displayID };
                        // window->on_display_content_scale_changed(id);
                        break;
                    }

                    // System events
                    case System::Event::ClipboardUpdate:
                        break;

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
        bool m_quit{ false };

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
