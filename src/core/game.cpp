#include <chrono>
#include <iostream>
#include <locale>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <flecs.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <fmt/std.h>

#include "core/game.hpp"
#include "core/numeric_types.hpp"
#include "core/options.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/benchmark_scene.hpp"
#include "ecs/scenes/main_menu_scene.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "sdl/tests/test_suite.hpp"
#include "sdl/time.hpp"
#include "utils/io.hpp"

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

        scene::main_menu::init(m_world);
        scene::benchmark::init(m_world, m_sdl.window());

        // restrict to one active scene at a time
        m_world.component<scene::active>().add(flecs::Exclusive);
        scene::set_active<scene::benchmark_scene>(m_world);
        runtime_assert(result, "failed to initialize game");

        if constexpr (EXECUTE_TESTS && result)
            result = this->run_tests(10);

        return result;
    }

    inline bool Game::quit_requested() const
    {
        return m_sdl.quit_triggered() ||  //
               m_world.should_quit();
    }

    bool Game::handle_events()
    {
        return m_sdl.handle_events();
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

        sdl::Color c_orange{ fmt::color::burly_wood };
        sdl::Timer<float, sdl::TimeDuration::Second> timer{};
        sdl::Color clear_clr1{ 0.2f, 0.3f, 0.3f, 1.0f };
        sdl::Color clear_clr2{ 0.7f, 0.4f, 0.4f, 1.0f };
        u32 loop_count = 0;
        auto delta_time_s = timer.delta();
        auto elapsed_time = timer.elapsed();

        sdl::Window& window = m_sdl.window();
        while (this->handle_events())
        {
            m_world.progress();
            if (this->quit_requested()) [[unlikely]]
                break;

            window.swap_buffers();
            window.renderer()->clear(clear_clr1);
            window.swap_buffers();
            window.renderer()->clear(clear_clr2);

            if constexpr (io::logging::main_loop && ++loop_count % 60 == 0)
            {
                elapsed_time = timer.elapsed();

                fmt::print(
                    c_orange,
                    fmt::format(
                        io::locale,
                        " {:>14.6f} s || {:>10L} u ][ {:>10.4f} ms | {:>10.4f} fps ][ {:>10.4f} avg fps ]\n",
                        elapsed_time,                 // elapsed time (seconds)
                        loop_count,                   // loop iterations
                        delta_time_s * 1000.0f,       // delta time (ms)
                        1.0f / delta_time_s,          // current fps
                        loop_count / elapsed_time));  // avg fps
            }

            delta_time_s = timer.delta();
        }

        return this->teardown();
    }

    sdl::Application& Game::sdl()
    {
        return m_sdl;
    }

    void Game::quit()
    {
        this->teardown();
    }

    bool Game::teardown()
    {
        scene::benchmark::deinit();
        return true;
    }

    // static bool benchmark()
    //{
    //     double elapsed{ 0 };
    //     u64 loop_count{ 0 };

    //    srand((u32)time(nullptr));
    //    auto& window{ m_sdl.window() };
    //    auto renderer{ window.renderer() };
    //    sdl::color color{ 0, 0, 0 };
    //    sdl::application::timer_t timer{};
    //    while (elapsed < 60000) [[unlikely]]
    //    {
    //        elapsed = timer.elapsed();

    //        renderer->clear();
    //        renderer->set_draw_color(color);
    //        renderer->present();

    //        if (loop_count++ % 960 == 0)
    //        {
    //            double dt = timer.elapsed() - elapsed;
    //            // const double avg_dlt{ ((elapsed_ms * 1000.0) / cast::to<f64>(loop_count)) };
    //            // const double avg_ups{ (cast::to<f64>(loop_count) / elapsed_ms) };
    //            fmt::print("[prev_dt={:<6.4f}ms] [avg_dlt={:<6.4f}ms] [avg_ups={:<6.4f}]\n", dt,
    //                       ((f64)loop_count / elapsed), (elapsed * 1000.0) / (f64)loop_count);
    //            color = {
    //                rand() % 128,
    //                rand() % 128,
    //                rand() % 128,
    //            };
    //        }
    //    }

    //    return true;
    //}
}
