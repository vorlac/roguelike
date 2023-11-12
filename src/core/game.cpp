#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <flecs.h>
#include <fmt/format.h>

#include "core/game.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/io.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/benchmark_scene.hpp"
#include "ecs/scenes/main_menu_scene.hpp"
#include "ecs/scenes/scene_types.hpp"
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

        scene::main_menu::init(m_world);
        scene::benchmark::init(m_world, m_sdl.window());

        // restrict to one active scene at a time
        m_world.component<scene::active>().add(flecs::Exclusive);
        scene::set_active<scene::benchmark_scene>(m_world);

        runtime_assert(result, "failed to initialize game");
        if (result)
        {
            if constexpr (EXECUTE_TESTS)
                result = this->run_tests(10);
        }

        return result;
    }

    inline bool Game::quit_requested() const
    {
        return false;
        // return m_world.should_quit();

        // SDL3::SDL_PumpEvents();
        // return SDL3::SDL_PeepEvents(nullptr, 0, SDL3::SDL_PEEKEVENT, SDL3::SDL_EVENT_QUIT,
        //                             SDL3::SDL_EVENT_QUIT) > 0;
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
    namespace sdl {
        struct hrtimer
        {
        private:
            uint64_t m_tick_frequency{ 0 };
            uint64_t m_start_hpc_tick{ 0 };
            uint64_t m_start_microsec{ 0 };
            uint64_t m_last_timestamp{ 0 };

        public:
            hrtimer()
            {
                LARGE_INTEGER tick_frequency{ 0 };
                BOOL ret = QueryPerformanceFrequency(&tick_frequency);
                if (ret == 0)
                    fmt::print("QueryPerformanceFrequency failed\n");

                LARGE_INTEGER start_hpc_tick{ 0 };
                ret = QueryPerformanceCounter(&start_hpc_tick);
                if (ret == 0)
                    fmt::print("QueryPerformanceCounter failed\n");

                LARGE_INTEGER current_tick{ 0 };
                ret = QueryPerformanceCounter(&current_tick);
                if (ret == 0)
                    fmt::print("QueryPerformanceCounter failed\n");

                m_tick_frequency = tick_frequency.QuadPart;
                m_start_hpc_tick = start_hpc_tick.QuadPart;
                m_start_microsec = ((current_tick.QuadPart - m_start_hpc_tick) * 1000000) /
                                   m_tick_frequency;
                m_last_timestamp = m_start_microsec;
            }

            [[nodiscard]]
            inline uint64_t now()
            {
                LARGE_INTEGER current_tick{ 0 };
                QueryPerformanceCounter(&current_tick);
                m_last_timestamp = current_tick.QuadPart;
                return m_last_timestamp;
            }

            /**
             * @brief get elapsed microseconds
             * */
            [[nodiscard]]
            inline uint64_t elapsed_mu()
            {
                uint64_t curr_timestamp = hrtimer::now();
                m_last_timestamp = ((curr_timestamp - m_start_hpc_tick) * 1000000) /
                                   m_tick_frequency;
                return m_last_timestamp;
            }

            /**
             * @brief get elapsed milliseconds
             * */
            [[nodiscard]]
            inline double elapsed_ms()
            {
                uint64_t microseconds = elapsed_mu();
                return static_cast<double>(microseconds) / 1000.0;
            }

            /**
             * @brief get elapsed seconds
             * */
            [[nodiscard]]
            inline double elapsed_sec()
            {
                double milliseconds = elapsed_ms();
                return milliseconds / 1000.0;
            }
        };
    }

    bool Game::run()
    {
        this->setup();

        u32 loop_count = 0;
        sdl::hrtimer timer{};
        while (m_world.progress())
        {
            if (++loop_count % 120 == 0)
            {
                // double seconds = timer.elapsed() * 1000.0;
                printf(fmt::format(io::locale,
                                   " {:>14.6f} s || {:>10L} u ][ {:>10.4f} ms ][ {:>10.4f} ups ]\n",
                                   timer.elapsed_sec(), loop_count, m_world.delta_time(),
                                   loop_count / timer.elapsed_sec())
                           .c_str());
            }
        }
        return this->teardown();
    }

    sdl::application& Game::sdl()
    {
        return m_sdl;
    }

    void Game::quit()
    {
        m_world.quit();
        this->teardown();
    }

    bool Game::teardown()
    {
        return true;
    }
}
