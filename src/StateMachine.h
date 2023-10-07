#pragma once

#include "State.h"
#include "core/Application.h"

#include <vector>

class StateMachine
{
private:
    std::vector<State*> states;
    Application* application;

public:
    StateMachine(Application* app)
    {
        application = app;
    }

    void Push(State* state)
    {
        state->OnPush();
        states.push_back(state);
    }

    State* Pop()
    {
        State* poppedState = states.back();
        states.pop_back();
        poppedState->OnPop();
        return poppedState;
    }

    State* Top()
    {
        return states.back();
    }

    int Count()
    {
        return static_cast<int>(states.size());
    }

    template <typename T>
    void PushNew()
    {
        T* toPush = new T(application);
        Push(toPush);
    }
};
