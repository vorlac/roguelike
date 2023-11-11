#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include "core/game.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/io.hpp"
#include "sdl/tests/test_renderer.hpp"

namespace rl {
    bool Game::setup()
    {
        bool result{ m_sdl.is_initialized() };
        runtime_assert(result, "failed to initialize game");
        if (!result)
            return result;

#if defined(ROGUELIKE_TESTS_ENABLED)
        return this->run_tests(100);
#endif
    }

    static bool quit_requested()
    {
        SDL3::SDL_PumpEvents();
        return SDL3::SDL_PeepEvents(nullptr, 0, SDL3::SDL_PEEKEVENT, SDL3::SDL_EVENT_QUIT,
                                    SDL3::SDL_EVENT_QUIT) > 0;
    }

    bool Game::run_tests(i32 iterations)
    {
        i32 ret = 0;
        i32 count = 0;
        while (count++ < iterations)
        {
            log::info("Running rendering tests [{}/{}]", count, iterations);
            ret |= sdl::test::execute_render_tests(m_sdl.window(), m_sdl.renderer());
        }
        return ret == 0;
    }

    bool Game::run()
    {
        auto&& timer = m_sdl.timer();

        this->setup();

        u64 i = 0;
        double delta_time = timer.delta();
        while (!quit_requested())
        {
            // m_sdl.run_sdl_tests();
            //   m_sdl.loop2();
            log::info("dt = {:<6.4L}ms", timer.delta());
            using namespace std::literals;
            std::this_thread::sleep_for(0.1s);
        }

        this->teardown();
        return true;
    }

    sdl::application& Game::sdl()
    {
        return m_sdl;
    }

    bool Game::teardown()
    {
        return true;
    }

    void Game::quit()
    {
    }
}
