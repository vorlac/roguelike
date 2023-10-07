#include "core/Application.h"

#include "StateMachine.h"
#include "states/MainMenuState.h"

#include <raylib.h>

void init_window()
{
    InitWindow(1280, 720, "CosmicDawn");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_VSYNC_HINT);
    TraceLog(LOG_INFO, "Startup");
}

Application::Application()
{
    init_window();
    running = true;
    lastTime = GetTime();
    clearColor = { 0, 0, 0 };
    stateMachine = new StateMachine(this);
    stateMachine->PushNew<MainMenuState>();
}

void Application::Run()
{
    while (running && !WindowShouldClose())
    {
        double currrentTime = GetTime();
        if (stateMachine->Count() > 0)
        {
            State* currentState = stateMachine->Top();

            // Update
            currentState->Update(currrentTime - lastTime);

            // Input
            bool shifted = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
            bool ctrld = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            bool alted = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);

            int key = GetKeyPressed();
            if (key != KEY_NULL && key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT
                && key != KEY_LEFT_CONTROL && key != KEY_RIGHT_CONTROL)
            {
                std::string action = settings.keyboard.EvaluateKey(
                    (KeyboardKey)key, currentState->GetKeyboardContext(), shifted, ctrld, alted);
                if (action != "none")
                {
                    Event event;
                    event.type = KeyboardEvent;
                    event.gameEvent = action;
                    currentState->OnEvent(event);
                    TraceLog(LOG_INFO, "did keyboard action: %s", action.c_str());
                }
            }

            // Draw
            BeginDrawing();
            ClearBackground(clearColor);
            currentState->Draw();
            EndDrawing();
            lastTime = currrentTime;
        }
        else
        {
            running = false;
        }
    }
    CloseWindow();
}

Settings Application::GetSettings()
{
    return settings;
}

StateMachine* Application::GetStateMachine()
{
    return stateMachine;
}
