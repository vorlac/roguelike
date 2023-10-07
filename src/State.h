#pragma once

#include "Event.h"

#include <entt/entt.hpp>
#include <raylib.h>
#include <string>

class Application;

class State
{
protected:
    bool debug;
    bool isLoaded;
    std::string name;
    std::string keyboardContextID;
    Application* application;

public:
    State(Application* app);
    virtual void OnEvent(Event event) = 0;
    virtual void OnUpdate(double delta) = 0;

    virtual void OnBeginDraw()
    {
    }

    virtual void OnDraw() = 0;
    virtual void OnEndDraw(){};
    virtual void OnDebugDraw();
    virtual void OnPush() = 0;
    virtual void OnPop() = 0;
    virtual void OnLoad();
    void Draw();
    void Update(double delta);
    std::string GetName();
    std::string GetKeyboardContext();
    void ToggleDebug();
};
