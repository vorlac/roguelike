#include "TestState.h"

TestState::TestState(Application* app)
    : State(app)
{
    keyboardContextID = "game";
    name = "TestState";
    font = LoadFontEx("data/fonts/Px437_IBM_EGA_9x14.ttf", 14, 0, 250);
}

void TestState::OnLoad()
{
    TraceLog(LOG_INFO, "TestState::OnLoad()");
    State::OnLoad();
}

void TestState::OnUpdate(double delta)
{
}

void TestState::OnDraw()
{
    Vector2 pp = { 190.0f, 220.0f };
    DrawTextEx(font, "Congrats! You created your first window!", pp, 14, 2, LIGHTGRAY);
}

void TestState::OnEvent(Event event)
{
    if (event.type == EventType::KeyboardEvent)
    {
        if (event.gameEvent == "debug")
            ToggleDebug();
    }
}

void TestState::OnPush()
{
}

void TestState::OnPop()
{
}
