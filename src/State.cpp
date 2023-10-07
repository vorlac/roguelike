
#include "State.h"

State::State(Application* app)
{
    TraceLog(LOG_INFO, "State::State()");
    debug = false;
    isLoaded = false;
    name = "State";
    application = app;
    entt::registry world;
}

void State::OnLoad()
{
    TraceLog(LOG_INFO, "State::OnLoad()");
    isLoaded = true;
}

void State::Draw()
{
    OnBeginDraw();
    OnDraw();
    if (debug == true)
        OnDebugDraw();
    OnEndDraw();
}

void State::OnDebugDraw()
{
    DrawFPS(5, 5);
    DrawText(name.c_str(), 5, 25, 20, LIGHTGRAY);
}

void State::Update(double delta)
{
    OnUpdate(delta);
}

std::string State::GetName()
{
    return name;
}

std::string State::GetKeyboardContext()
{
    return keyboardContextID;
}

void State::ToggleDebug()
{
    debug = !(debug);
}
