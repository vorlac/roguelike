#pragma once

#include <string>

enum EventType
{
    None = 0,
    // Keyboard
    KeyboardEvent,
    KeyPress,
    KeyRelease,
    // Mouse
    MouseEvent,
    MouseClick,
    MouseRelease,
    // Windowing
    WindowClose,
    WindowFocus,
    WindowLoseFocus,
    WindowMinimize,
    WindowMaximize,
    // Higher level event
    GameEvent
};

struct Event
{
    EventType type;
    char Key;
    int Keycode;
    int x;
    int y;
    int button;
    std::string gameEvent;
};
