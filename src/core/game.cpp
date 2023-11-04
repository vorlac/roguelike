#include <chrono>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/game.hpp"
#include "core/numeric_types.hpp"

namespace rl
{
    bool Game::setup()
    {
        return m_sdl.is_initialized();
    }

    bool Game::run()
    {
        this->setup();

        u64 i = 0;
        while (true)
        {
            ++i;
            fmt::print("{}\n", i);
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

    bool Game::should_quit() const
    {
        return false;
    }

    void Game::quit()
    {
    }
}
