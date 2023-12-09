#pragma once

#include "core/state/gamestate.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl {
    class TeardownState : public GameState
    {
        void enter()
        {
            log::info("TeardownState::enter()");
        }

        void process_event(SDL3::SDL_Event&& e)
        {
            log::info("TeardownState::process_event()");
            switch (e.type)
            {
                case 1025:
                    break;
            }
        }

        void exit()
        {
            log::info("TeardownState::exit()");
        }
    };

    class PauseMenuState : public GameState
    {
        void enter()
        {
            log::info("PauseMenuState::enter()");
        }

        void process_event(SDL3::SDL_Event&& e)
        {
            switch (e.type)
            {
                case 1025:
                    break;
            }
        }

        void exit()
        {
            log::info("PauseMenuState::exit()");
        }
    };

    class GameplayState : public GameState
    {
        void enter()
        {
            log::info("GameplayState::enter()");
        }

        void process_event(SDL3::SDL_Event&& e)
        {
            switch (e.type)
            {
                case 1025:
                    break;
            }
        }

        void exit()
        {
            log::info("GameplayState::exit()");
        }
    };

    class LoadLevelState : public GameState
    {
        void enter()
        {
            log::info("LoadLevelState::enter()");
        }

        void process_event(SDL3::SDL_Event&& e)
        {
            log::info("LoadLevelState::process_event()");
            switch (e.type)
            {
                case 1025:
                    break;
            }
        }

        void exit()
        {
            log::info("LoadLevelState::exit()");
        }
    };

    class MainMenuState : public GameState
    {
        void enter()
        {
            log::info("MainMenuState::enter()");
        }

        void process_event(SDL3::SDL_Event&& e)
        {
            log::info("MainMenuState::process_event()");
            switch (e.type)
            {
                case 1025:
                    break;
            }
        }

        void exit()
        {
            log::info("MainMenuState::exit()");
        }
    };

    class GameInitState : public GameState
    {
        void enter()
        {
            log::info("GameInitState::enter()");
        }

        void process_event(auto&&, StateMachine& fsm)
        {
            fsm.push(std::make_unique<MainMenuState>());
        }

        void exit()
        {
            log::info("GameInitState::exit()");
        }
    };
}
