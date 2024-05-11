#pragma once

#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "core/system.hpp"
#include "utils/sdl_defs.hpp"

namespace rl {
    class EventHandler
    {
        constexpr static i32 resizing_event_watcher(void* data, SDL3::SDL_Event* e)
        {
            if (e->type != MainWindow::Event::Resized)
                return 0;

            const auto window{ static_cast<MainWindow*>(data) };
            window->window_resized_event_callback(*e);
            return 1;
        }

    public:
        constexpr EventHandler() = default;

        explicit EventHandler(const std::unique_ptr<MainWindow>& window)
        {
            SDL3::SDL_AddEventWatch(resizing_event_watcher, window.get());
        }

        bool handle_events(const std::unique_ptr<MainWindow>& window)
        {
            SDL3::SDL_Event e{};
            while (SDL3::SDL_PollEvent(&e) != 0) {
                switch (e.type) {
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

                    case Keyboard::Event::KeyDown:
                    {
                        window->keyboard_key_pressed_event_callback(e);
                        const auto key{ static_cast<Keyboard::Scancode>(e.key.keysym.scancode) };
                        if (key == Keyboard::Scancode::Escape) [[unlikely]]
                            m_quit = true;
                        break;
                    }
                    case Keyboard::Event::KeyUp:
                        window->keyboard_key_released_event_callback(e);
                        break;

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

                    case MainWindow::DisplayEvent::ContentScaleChanged:
                        break;

                    case System::Event::ClipboardUpdate:
                        break;

                    case Event::Quit:
                        m_quit = true;
                        break;

                    default:
                        break;
                }
            }
            return true;
        }

        constexpr bool quit_triggered() const
        {
            return m_quit;
        }

    private:
        bool m_quit{ false };

    public:
        enum ButtonState {
            Pressed = 0,
            Released = 1,
        };

        struct EventAction
        {
            using type = SDL3::SDL_EventAction;
            constexpr static type Add = SDL3::SDL_ADDEVENT;
            constexpr static type Peek = SDL3::SDL_PEEKEVENT;
            constexpr static type Get = SDL3::SDL_GETEVENT;
        };

        struct Event
        {
            using type = SDL3::SDL_EventType;
            constexpr static type First = SDL3::SDL_EVENT_FIRST;
            constexpr static type Quit = SDL3::SDL_EVENT_QUIT;
            constexpr static type Terminating = SDL3::SDL_EVENT_TERMINATING;
            constexpr static type LowMemory = SDL3::SDL_EVENT_LOW_MEMORY;
            constexpr static type WillEnterBackground = SDL3::SDL_EVENT_WILL_ENTER_BACKGROUND;
            constexpr static type DidEnterBackground = SDL3::SDL_EVENT_DID_ENTER_BACKGROUND;
            constexpr static type WillEnterForeground = SDL3::SDL_EVENT_WILL_ENTER_FOREGROUND;
            constexpr static type DidEnterForeground = SDL3::SDL_EVENT_DID_ENTER_FOREGROUND;
            constexpr static type LocaleChanged = SDL3::SDL_EVENT_LOCALE_CHANGED;
            constexpr static type SystemThemeChanged = SDL3::SDL_EVENT_SYSTEM_THEME_CHANGED;

            constexpr static type DisplayOrientation = SDL3::SDL_EVENT_DISPLAY_ORIENTATION;
            constexpr static type DisplayAdded = SDL3::SDL_EVENT_DISPLAY_ADDED;
            constexpr static type DisplayRemoved = SDL3::SDL_EVENT_DISPLAY_REMOVED;
            constexpr static type DisplayMoved = SDL3::SDL_EVENT_DISPLAY_MOVED;
            constexpr static type DispContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED;
            constexpr static type DisplayFirst = SDL3::SDL_EVENT_DISPLAY_FIRST;
            constexpr static type DisplayLast = SDL3::SDL_EVENT_DISPLAY_LAST;

            constexpr static type JoystickAxisMotion = SDL3::SDL_EVENT_JOYSTICK_AXIS_MOTION;
            constexpr static type JoystickHatMotion = SDL3::SDL_EVENT_JOYSTICK_HAT_MOTION;
            constexpr static type JoystickButtonDown = SDL3::SDL_EVENT_JOYSTICK_BUTTON_DOWN;
            constexpr static type JoystickButtonUp = SDL3::SDL_EVENT_JOYSTICK_BUTTON_UP;
            constexpr static type JoystickAdded = SDL3::SDL_EVENT_JOYSTICK_ADDED;
            constexpr static type JoystickRemoved = SDL3::SDL_EVENT_JOYSTICK_REMOVED;
            constexpr static type JoystickBatteryUpdated = SDL3::SDL_EVENT_JOYSTICK_BATTERY_UPDATED;
            constexpr static type JoystickUpdateComplete = SDL3::SDL_EVENT_JOYSTICK_UPDATE_COMPLETE;

            constexpr static type GamepadAxisMotion = SDL3::SDL_EVENT_GAMEPAD_AXIS_MOTION;
            constexpr static type GamepadButtonDown = SDL3::SDL_EVENT_GAMEPAD_BUTTON_DOWN;
            constexpr static type GamepadButtonUp = SDL3::SDL_EVENT_GAMEPAD_BUTTON_UP;
            constexpr static type GamepadAdded = SDL3::SDL_EVENT_GAMEPAD_ADDED;
            constexpr static type GamepadRemoved = SDL3::SDL_EVENT_GAMEPAD_REMOVED;
            constexpr static type GamepadRemapped = SDL3::SDL_EVENT_GAMEPAD_REMAPPED;
            constexpr static type GamepadTouchpadDown = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN;
            constexpr static type GamepadTouchpadMotion = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION;
            constexpr static type GamepadTouchpadUp = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_UP;
            constexpr static type GamepadSensorUpdate = SDL3::SDL_EVENT_GAMEPAD_SENSOR_UPDATE;
            constexpr static type GamepadUpdateComplete = SDL3::SDL_EVENT_GAMEPAD_UPDATE_COMPLETE;

            constexpr static type FingerDown = SDL3::SDL_EVENT_FINGER_DOWN;
            constexpr static type FingerUp = SDL3::SDL_EVENT_FINGER_UP;
            constexpr static type FingerMotion = SDL3::SDL_EVENT_FINGER_MOTION;

            constexpr static type ClipboardUpdate = SDL3::SDL_EVENT_CLIPBOARD_UPDATE;
            constexpr static type DropFile = SDL3::SDL_EVENT_DROP_FILE;
            constexpr static type DropText = SDL3::SDL_EVENT_DROP_TEXT;
            constexpr static type DropBegin = SDL3::SDL_EVENT_DROP_BEGIN;
            constexpr static type DropComplete = SDL3::SDL_EVENT_DROP_COMPLETE;
            constexpr static type DropPosition = SDL3::SDL_EVENT_DROP_POSITION;

            constexpr static type AudioDeviceAdded = SDL3::SDL_EVENT_AUDIO_DEVICE_ADDED;
            constexpr static type AudioDeviceRemoved = SDL3::SDL_EVENT_AUDIO_DEVICE_REMOVED;
            constexpr static type AudioDeviceFormatChanged = SDL3::SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED;

            constexpr static type SensorUpdate = SDL3::SDL_EVENT_SENSOR_UPDATE;
            constexpr static type RenderTargetsReset = SDL3::SDL_EVENT_RENDER_TARGETS_RESET;
            constexpr static type RenderDeviceReset = SDL3::SDL_EVENT_RENDER_DEVICE_RESET;
            constexpr static type PollSentinel = SDL3::SDL_EVENT_POLL_SENTINEL;
            constexpr static type User = SDL3::SDL_EVENT_USER;

            constexpr static type Last = SDL3::SDL_EVENT_LAST;
        };
    };
}
