#pragma once

namespace SDL3 {
#include <SDL3/SDL_events.h>
}

namespace rl::sdl {
    class event_handler
    {
    public:
        enum ButtonState {
            Pressed = 0,
            Released = 1,
        };

    public:
        auto handle_inputs()
        {
            SDL3::SDL_Event e{};

            // Handle any queued events
            while (SDL3::SDL_PollEvent(&e) != 0)
            {
                // User requests quit
                if (e.type == SDL_QUIT)
                    quit = true;
            }
        }

    private:
        bool m_quit = false;

    public:
        struct EventID
        {
            using sdl_type = SDL3::SDL_EventType;
            constexpr sdl_type first = SDL3::SDL_EVENT_FIRST;
            constexpr sdl_type quit = SDL3::SDL_EVENT_QUIT;
            constexpr sdl_type terminating = SDL3::SDL_EVENT_TERMINATING;
            constexpr sdl_type LowMemory = SDL3::SDL_EVENT_LOW_MEMORY;
            constexpr sdl_type WillEnterBackground = SDL3::SDL_EVENT_WILL_ENTER_BACKGROUND;
            constexpr sdl_type DidEnterBackground = SDL3::SDL_EVENT_DID_ENTER_BACKGROUND;
            constexpr sdl_type WillEnterForeground = SDL3::SDL_EVENT_WILL_ENTER_FOREGROUND;
            constexpr sdl_type DidEnterForeground = SDL3::SDL_EVENT_DID_ENTER_FOREGROUND;
            constexpr sdl_type LocaleChanged = SDL3::SDL_EVENT_LOCALE_CHANGED;
            constexpr sdl_type SystemThemeChanged = SDL3::SDL_EVENT_SYSTEM_THEME_CHANGED;
            constexpr sdl_type DisplayOrientation = SDL3::SDL_EVENT_DISPLAY_ORIENTATION;
            constexpr sdl_type DisplayAdded = SDL3::SDL_EVENT_DISPLAY_ADDED;
            constexpr sdl_type DisplayRemoved = SDL3::SDL_EVENT_DISPLAY_REMOVED;
            constexpr sdl_type DisplayMoved = SDL3::SDL_EVENT_DISPLAY_MOVED;
            constexpr sdl_type
                DisplayContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED;
            constexpr sdl_type DisplayFirst = SDL3::SDL_EVENT_DISPLAY_FIRST;
            constexpr sdl_type DisplayLast = SDL3::SDL_EVENT_DISPLAY_LAST;
            constexpr sdl_type syswm = SDL3::SDL_EVENT_SYSWM;
            constexpr sdl_type WindowShown = SDL3::SDL_EVENT_WINDOW_SHOWN;
            constexpr sdl_type WindowHidden = SDL3::SDL_EVENT_WINDOW_HIDDEN;
            constexpr sdl_type WindowExposed = SDL3::SDL_EVENT_WINDOW_EXPOSED;
            constexpr sdl_type WindowMoved = SDL3::SDL_EVENT_WINDOW_MOVED;
            constexpr sdl_type WindowResized = SDL3::SDL_EVENT_WINDOW_RESIZED;
            constexpr sdl_type WindowPixelSizeChanged = SDL3::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED;
            constexpr sdl_type WindowMinimized = SDL3::SDL_EVENT_WINDOW_MINIMIZED;
            constexpr sdl_type WindowMaximized = SDL3::SDL_EVENT_WINDOW_MAXIMIZED;
            constexpr sdl_type WindowRestored = SDL3::SDL_EVENT_WINDOW_RESTORED;
            constexpr sdl_type WindowMouseEnter = SDL3::SDL_EVENT_WINDOW_MOUSE_ENTER;
            constexpr sdl_type WindowMouseLeave = SDL3::SDL_EVENT_WINDOW_MOUSE_LEAVE;
            constexpr sdl_type WindowFocusGained = SDL3::SDL_EVENT_WINDOW_FOCUS_GAINED;
            constexpr sdl_type WindowFocusLost = SDL3::SDL_EVENT_WINDOW_FOCUS_LOST;
            constexpr sdl_type WindowCloseRequested = SDL3::SDL_EVENT_WINDOW_CLOSE_REQUESTED;
            constexpr sdl_type WindowTakeFocus = SDL3::SDL_EVENT_WINDOW_TAKE_FOCUS;
            constexpr sdl_type WindowHitTest = SDL3::SDL_EVENT_WINDOW_HIT_TEST;
            constexpr sdl_type WindowIccprofChanged = SDL3::SDL_EVENT_WINDOW_ICCPROF_CHANGED;
            constexpr sdl_type WindowDisplayChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_CHANGED;
            constexpr sdl_type
                WindowDisplayScaleChanged = SDL3::SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED;
            constexpr sdl_type WindowOccluded = SDL3::SDL_EVENT_WINDOW_OCCLUDED;
            constexpr sdl_type WindowDestroyed = SDL3::SDL_EVENT_WINDOW_DESTROYED;
            constexpr sdl_type WindowFirst = SDL3::SDL_EVENT_WINDOW_FIRST;
            constexpr sdl_type WindowLast = SDL3::SDL_EVENT_WINDOW_LAST;
            constexpr sdl_type KeyDown = SDL3::SDL_EVENT_KEY_DOWN;
            constexpr sdl_type KeyUp = SDL3::SDL_EVENT_KEY_UP;
            constexpr sdl_type TextEditing = SDL3::SDL_EVENT_TEXT_EDITING;
            constexpr sdl_type TextInput = SDL3::SDL_EVENT_TEXT_INPUT;
            constexpr sdl_type KeymapChanged = SDL3::SDL_EVENT_KEYMAP_CHANGED;
            constexpr sdl_type TextEditingExt = SDL3::SDL_EVENT_TEXT_EDITING_EXT;
            constexpr sdl_type MouseMotion = SDL3::SDL_EVENT_MOUSE_MOTION;
            constexpr sdl_type MouseButtonDown = SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN;
            constexpr sdl_type MouseButtonUp = SDL3::SDL_EVENT_MOUSE_BUTTON_UP;
            constexpr sdl_type MouseWheel = SDL3::SDL_EVENT_MOUSE_WHEEL;
            constexpr sdl_type JoystickAxisMotion = SDL3::SDL_EVENT_JOYSTICK_AXIS_MOTION;
            constexpr sdl_type JoystickHatMotion = SDL3::SDL_EVENT_JOYSTICK_HAT_MOTION;
            constexpr sdl_type JoystickButtonDown = SDL3::SDL_EVENT_JOYSTICK_BUTTON_DOWN;
            constexpr sdl_type JoystickButtonUp = SDL3::SDL_EVENT_JOYSTICK_BUTTON_UP;
            constexpr sdl_type JoystickAdded = SDL3::SDL_EVENT_JOYSTICK_ADDED;
            constexpr sdl_type JoystickRemoved = SDL3::SDL_EVENT_JOYSTICK_REMOVED;
            constexpr sdl_type JoystickBatteryUpdated = SDL3::SDL_EVENT_JOYSTICK_BATTERY_UPDATED;
            constexpr sdl_type JoystickUpdateComplete = SDL3::SDL_EVENT_JOYSTICK_UPDATE_COMPLETE;
            constexpr sdl_type GamepadAxisMotion = SDL3::SDL_EVENT_GAMEPAD_AXIS_MOTION;
            constexpr sdl_type GamepadButtonDown = SDL3::SDL_EVENT_GAMEPAD_BUTTON_DOWN;
            constexpr sdl_type GamepadButtonUp = SDL3::SDL_EVENT_GAMEPAD_BUTTON_UP;
            constexpr sdl_type GamepadAdded = SDL3::SDL_EVENT_GAMEPAD_ADDED;
            constexpr sdl_type GamepadRemoved = SDL3::SDL_EVENT_GAMEPAD_REMOVED;
            constexpr sdl_type GamepadRemapped = SDL3::SDL_EVENT_GAMEPAD_REMAPPED;
            constexpr sdl_type GamepadTouchpadDown = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN;
            constexpr sdl_type GamepadTouchpadMotion = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION;
            constexpr sdl_type GamepadTouchpadUp = SDL3::SDL_EVENT_GAMEPAD_TOUCHPAD_UP;
            constexpr sdl_type GamepadSensorUpdate = SDL3::SDL_EVENT_GAMEPAD_SENSOR_UPDATE;
            constexpr sdl_type GamepadUpdateComplete = SDL3::SDL_EVENT_GAMEPAD_UPDATE_COMPLETE;
            constexpr sdl_type FingerDown = SDL3::SDL_EVENT_FINGER_DOWN;
            constexpr sdl_type FingerUp = SDL3::SDL_EVENT_FINGER_UP;
            constexpr sdl_type FingerMotion = SDL3::SDL_EVENT_FINGER_MOTION;
            constexpr sdl_type ClipboardUpdate = SDL3::SDL_EVENT_CLIPBOARD_UPDATE;
            constexpr sdl_type DropFile = SDL3::SDL_EVENT_DROP_FILE;
            constexpr sdl_type DropText = SDL3::SDL_EVENT_DROP_TEXT;
            constexpr sdl_type DropBegin = SDL3::SDL_EVENT_DROP_BEGIN;
            constexpr sdl_type DropComplete = SDL3::SDL_EVENT_DROP_COMPLETE;
            constexpr sdl_type DropPosition = SDL3::SDL_EVENT_DROP_POSITION;
            constexpr sdl_type AudioDeviceAdded = SDL3::SDL_EVENT_AUDIO_DEVICE_ADDED;
            constexpr sdl_type AudioDeviceRemoved = SDL3::SDL_EVENT_AUDIO_DEVICE_REMOVED;
            constexpr sdl_type AudioDeviceFormatChanged = SDL3::SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED;
            constexpr sdl_type SensorUpdate = SDL3::SDL_EVENT_SENSOR_UPDATE;
            constexpr sdl_type RenderTargetsReset = SDL3::SDL_EVENT_RENDER_TARGETS_RESET;
            constexpr sdl_type RenderDeviceReset = SDL3::SDL_EVENT_RENDER_DEVICE_RESET;
            constexpr sdl_type PollSentinel = SDL3::SDL_EVENT_POLL_SENTINEL;
            constexpr sdl_type User = SDL3::SDL_EVENT_USER;
            constexpr sdl_type Last = SDL3::SDL_EVENT_LAST;
            // clang-format on
        };
    };

}
