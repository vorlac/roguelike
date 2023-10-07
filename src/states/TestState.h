#pragma once

#include "raylib.h"
#include "State.h"


class Application;

class TestState : public State
{
    private:
        Font font;
        
    public:
        TestState(Application *app);
        void OnLoad() override;
        void OnUpdate(double delta) override;
        void OnDraw() override;
        void OnEvent(Event event) override;
        void OnPush() override;
        void OnPop() override;
};