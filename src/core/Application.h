#pragma once

#include "ContentLoader.h"
#include "Settings.h"

#include <raylib.h>

class StateMachine;

class Application
{
protected:
    bool running;
    double lastTime;
    Color clearColor;
    StateMachine* stateMachine;

    Settings settings;
    ContentLoader content;

public:
    Application();
    void Run();
    Settings GetSettings();
    StateMachine* GetStateMachine();
};
