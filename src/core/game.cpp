#include "core/game.hpp"

#include <chrono>
#include <string>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/application.hpp"
#include "core/input/keymap.hpp"
#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ecs/components.hpp"
#include "ecs/observers.hpp"
#include "ecs/scenes.hpp"
#include "ecs/systems.hpp"
#include "utils/color.hpp"
#include "utils/io.hpp"
#include "utils/time.hpp"

namespace rl
{
    namespace c
    {
        struct position
        {
            float x{ 0 };
            float y{ 0 };
        };

        struct velocity
        {
            float x{ 0 };
            float y{ 0 };
        };

        struct style
        {
            Color color{ rl::color::lime };
        };
    }

    bool Game::teardown()
    {
        m_world.quit();
        return true;
    }

    bool Game::init()
    {
        const auto generate_world_entities = [&](uint32_t count) {
            const ds::position centroid{ m_window.center() };

            SetRandomSeed(1111111);
            for (size_t i = 0; i < count; ++i)
            {
                Color color{ rand_color(GetRandomValue(0, 64)) };
                std::string name{ fmt::format("Ball {}", i) };
                ds::velocity velocity{
                    static_cast<float>(GetRandomValue(-500, 500)) / 1.0f,
                    static_cast<float>(GetRandomValue(-500, 500)) / 1.0f,
                };

                m_world.entity(name.data())
                    .set<c::position>({ centroid.x, centroid.y })
                    .set<c::velocity>({ velocity.x, velocity.y })
                    .set<c::style>({ color });
            }

            return m_world.count<c::position>();
        };

        rl::timer timer{ "Entity creation time" };
        return timer.measure(generate_world_entities, 10000);
    }

    bool Game::run()
    {
        this->init();

        auto update_count{ 0 };
        rl::timer delta_timer{ "delta_time" };
        while (!this->should_quit())
        {
            float delta_time{ this->delta_time() };

            this->update(delta_time);
            this->render(delta_time);

            // clang-format off
            ++update_count % 60 == 0
                ? delta_timer.print_delta_time()
                : delta_timer.delta_update();
            // clang-format on
        }

        return 0;
    }

    void Game::update(float delta_time)
    {
        auto&& input_actions = m_input.active_game_actions();
#ifdef _DEBUG
        if (input_actions.size() > 0)
            for (auto&& action : input_actions)
                log::info("Input: {}", action);
#endif

        const auto window_size{ this->m_window.render_size() };

        auto top_bottom_collision = [&](const c::position& p) {
            bool top_collision = p.y - (rect_size.height / 2.0) <= 0.0;
            bool bottom_collision = p.y + (rect_size.height / 2.0) >= window_size.height;
            return top_collision || bottom_collision;
        };

        auto left_right_collision = [&](const c::position& p) {
            bool left_collision = p.x - (rect_size.width / 2.0) <= 0.0;
            bool right_collision = p.x + (rect_size.width / 2.0) >= window_size.width;
            return left_collision || right_collision;
        };

        m_world.each([&](flecs::entity, c::position& p, c::velocity& v, c::style&) {
            p.x += v.x * delta_time;
            p.y += v.y * delta_time;

            if (left_right_collision(p))
                v.x = -v.x;
            if (top_bottom_collision(p))
                v.y = -v.y;
        });

        m_world.progress(delta_time);
    }

    void Game::render(float)
    {
        m_window.render([&]() {
            ClearBackground(color::lightgray);

            m_world.each([](const c::position& p, const c::velocity&, const c::style& s) {
                DrawRectangle(
                    static_cast<int32_t>(p.x) - static_cast<int32_t>(rect_size.width / 2.0),
                    static_cast<int32_t>(p.y) - static_cast<int32_t>(rect_size.height / 2.0),
                    static_cast<int32_t>(rect_size.width), static_cast<int>(rect_size.height),
                    s.color);
            });

            DrawFPS(10, 10);
        });
    }

    bool Game::should_quit() const
    {
        return m_world.should_quit() || m_window.should_close();
    }

    void Game::quit() const
    {
        m_world.quit();
    }
}
