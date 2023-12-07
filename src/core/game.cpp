#include <algorithm>
#include <cctype>
#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <locale>
#include <memory>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <flecs.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <fmt/std.h>

#include "core/game.hpp"
#include "core/numeric.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/triangle.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/benchmark_scene.hpp"
#include "ecs/scenes/main_menu_scene.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "gl/instanced_buffer.hpp"
#include "gl/shader.hpp"
#include "gl/vertex_buffer.hpp"
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

        sdl::Timer<f32, sdl::TimeDuration::Second> timer{};

        u32 loop_count = 0;
        auto delta_time_s = timer.delta();
        auto elapsed_time = timer.elapsed();

        sdl::Window& window{ m_sdl.window() };
        std::shared_ptr<sdl::RendererGL> renderer{ window.renderer() };

        gl::VertexBuffer vbo{ renderer->get_viewport() };

        // gl::InstancedVertexBuffer vbo{ renderer->get_viewport() };
        vbo.bind_buffers();

        while (this->handle_events())
        {
            m_world.progress();
            renderer->clear();

            if (this->quit_requested()) [[unlikely]]
                break;

            // vbo.update_buffers(renderer->get_viewport());
            vbo.draw_triangles();

            window.swap_buffers();

            if constexpr (io::logging::main_loop)
                if (++loop_count % 60 == 0)
                {
                    elapsed_time = timer.elapsed();
                    log::debug(
                        " {:>14.6f} s || {:>10L} u ][ {:>10.4f} ms | {:>10.4f} fps ][ {:>10.4f} avg fps ]",
                        elapsed_time,                // elapsed time (seconds)
                        loop_count,                  // loop iterations
                        delta_time_s * 1000.0f,      // delta time (ms)
                        1.0f / delta_time_s,         // current fps
                        loop_count / elapsed_time);  // avg fps
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
}
