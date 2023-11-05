#include <chrono>
#include <memory>
#include <string>

#include "core/game.hpp"
#include "core/numeric_types.hpp"

namespace rl
{
    bool Game::setup()
    {
        return m_sdl.is_initialized();
    }

    static bool quit_requested()
    {
        SDL3::SDL_PumpEvents();
        return SDL3::SDL_PeepEvents(nullptr, 0, SDL3::SDL_PEEKEVENT, SDL3::SDL_EVENT_QUIT,
                                    SDL3::SDL_EVENT_QUIT) > 0;
    }

    bool Game::run()
    {
        this->setup();

        u64 i = 0;
        while (!quit_requested())
        {
            m_sdl.loop2();
            fmt::print("{}\n", ++i);
            using namespace std::literals;
            std::this_thread::sleep_for(0.1s);
        }

        this->teardown();
        return true;
    }

    bool Game::teardown()
    {
        return true;
    }

    void Game::quit()
    {
    }
}
