#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <fmt/format.h>

#include "core/game.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/io.hpp"
#include "sdl/tests/test_suite.hpp"
#include "sdl/time.hpp"

#undef ROGUELIKE_TESTS_ENABLED
#ifdef ROGUELIKE_TESTS_ENABLED
constexpr static bool EXECUTE_TESTS = true;
#else
constexpr static bool EXECUTE_TESTS = false;
#endif

namespace rl {
    bool Game::setup()
    {
        std::ios::ios_base::sync_with_stdio(false);
        bool result = m_sdl.is_initialized();
        runtime_assert(result, "failed to initialize game");
        if (result)
        {
            if constexpr (EXECUTE_TESTS)
                result = this->run_tests(10);
        }

        return result;
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
            ret |= sdl::test::execute_render_tests(m_sdl.window());
        }

        count = 0;
        while (count++ < iterations)
        {
            log::info("Running sprite drawing tests [{}/{}]", count, iterations);
            ret |= sdl::test::execute_sprite_drawing_tests(m_sdl.window());
        }

        return ret == 0;
    }

    bool Game::run()
    {
        this->setup();

        double elapsed{ 0 };
        u64 loop_count{ 0 };

        srand((u32)time(nullptr));
        auto& window{ m_sdl.window() };
        auto renderer{ window.renderer() };
        sdl::color color{ 0, 0, 0 };
        sdl::application::timer_t timer{};
        while (elapsed < 60000) [[unlikely]]
        {
            elapsed = timer.elapsed();

            renderer->clear();
            renderer->set_draw_color(color);
            renderer->present();

            if (loop_count++ % 960 == 0)
            {
                double dt = timer.elapsed() - elapsed;
                // const double avg_dlt{ ((elapsed_ms * 1000.0) / cast::to<f64>(loop_count)) };
                // const double avg_ups{ (cast::to<f64>(loop_count) / elapsed_ms) };
                fmt::print("[prev_dt={:<6.4f}ms] [avg_dlt={:<6.4f}ms] [avg_ups={:<6.4f}]\n", dt,
                           ((f64)loop_count / elapsed), (elapsed * 1000.0) / (f64)loop_count);
                color = {
                    rand() % 128,
                    rand() % 128,
                    rand() % 128,
                };
            }
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
