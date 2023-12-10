#pragma once

#include "sdl/defs.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl {
    template <typename T>
    concept GameStateFull = requires(T& s) {
        s.enter();
        s.handle_event();
        s.update();
        s.render();
        s.exit();

        s.debug();
        s.log();
    };

    template <typename T>
    concept GameStateMin = requires(T& s) {
        s.enter();
        s.handle_event();
        s.update();
        s.render();
        s.exit();
    };

    template <typename T>
    concept IGameState = (GameStateMin<T> || GameStateFull<T>);

    template <typename TState>
    struct GameState
    {
        auto enter_state() const noexcept
        {
            auto& self = *static_cast<const TState*>(this);
            return self.enter();
        }

        auto process_events(auto&& e) const noexcept
        {
            auto& self = *static_cast<const TState*>(this);
            return self.handle_event();
        }

        auto update_state() const noexcept
        {
            auto& self = *static_cast<const TState*>(this);
            if constexpr (GameStateFull<TState>)
                self.log();
            return self.update();
        }

        auto render_state() const noexcept
        {
            auto& self = *static_cast<const TState*>(this);
            if constexpr (GameStateFull<TState>)
                self.debug();
            return self.render();
        }

        auto exit_state() const noexcept
        {
            auto& self = *static_cast<const TState*>(this);
            return self.exit();
        }
    };

    struct TeardownState : GameState<TeardownState>
    {
        auto enter() noexcept
        {
            return 0;
        }

        auto handle_event() noexcept
        {
            return 0;
        }

        auto update() noexcept
        {
            return 0;
        }

        auto render() noexcept
        {
            return 0;
        }

        auto exit() noexcept
        {
            return 0;
        }
    };

    struct PauseMenuState : GameState<PauseMenuState>
    {
        auto enter() noexcept
        {
            return 0;
        }

        auto handle_event() noexcept
        {
            return 0;
        }

        auto update() noexcept
        {
            return 0;
        }

        auto render() noexcept
        {
            return 0;
        }

        auto exit() noexcept
        {
            return 0;
        }
    };

    struct GameplayState : GameState<GameplayState>
    {
        auto enter() noexcept
        {
            return 0;
        }

        auto handle_event() noexcept
        {
            return 0;
        }

        auto update() noexcept
        {
            return 0;
        }

        auto render() noexcept
        {
            return 0;
        }

        auto exit() noexcept
        {
            return 0;
        }

        auto debug() noexcept
        {
            return 0;
        }

        auto log() noexcept
        {
            return 0;
        }
    };

    struct LoadLevelState : GameState<LoadLevelState>
    {
        auto enter() noexcept
        {
            return 0;
        }

        auto handle_event() noexcept
        {
            return 0;
        }

        auto update() noexcept
        {
            return 0;
        }

        auto render() noexcept
        {
            return 0;
        }

        auto exit() noexcept
        {
            return 0;
        }
    };

    struct MainMenuState : GameState<MainMenuState>
    {
        auto enter() noexcept
        {
            return 0;
        }

        auto handle_event() noexcept
        {
            return 0;
        }

        auto update() noexcept
        {
            return 0;
        }

        auto render() noexcept
        {
            return 0;
        }

        auto exit() noexcept
        {
            return 0;
        }

        auto debug() noexcept
        {
            return 0;
        }

        auto log() noexcept
        {
            return 0;
        }
    };

    struct GameInitState : GameState<GameInitState>
    {
        auto enter() noexcept
        {
            return 0;
        }

        auto handle_event() noexcept
        {
            return 0;
        }

        auto update() noexcept
        {
            return 0;
        }

        auto render() noexcept
        {
            return 0;
        }

        auto exit() noexcept
        {
            return 0;
        }

        auto debug() noexcept
        {
            return 0;
        }

        auto log() noexcept
        {
            return 0;
        }
    };
}
