#include "core/game.hpp"

#include <chrono>
#include <string>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/application.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/input/keymap.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/demo_scene.hpp"
#include "ecs/scenes/menu_scene.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "utils/color.hpp"
#include "utils/io.hpp"
#include "utils/time.hpp"

namespace rl
{
    bool Game::setup()
    {
        Application::setup();
        // Can only have one active scene in a game at a time.
        m_world.component<scene::active>().add(flecs::Exclusive);

        scene::init_demo_scene(m_world);
        scene::init_main_menu_scene(m_world);

        scene::set_active<scene::demo_level>(m_world);

        return true;
    }

    bool Game::teardown()
    {
        m_world.quit();
        Application::teardown();
        return true;
    }

    bool Game::run()
    {
        this->setup();

        auto update_count{ 0 };
        rl::timer delta_timer{ "delta_time" };
        while (!this->should_quit()) [[unlikely]]
        {
            float delta_time{ this->delta_time() };

            this->update(delta_time);
            this->render(delta_time);

            // m_world.progress(delta_time);

            ++update_count % 60 == 0              // every 60 updates
                ? delta_timer.print_delta_time()  // output delta_time
                : delta_timer.delta_update();     // update delta checkpoint
        }

        return 0;
    }

    void Game::update(float delta_time)
    {
        auto&& input_actions = m_input.active_game_actions();
        if (input_actions.size() > 0) [[unlikely]]
            for (auto&& action : input_actions)
                log::info("Input: {}", action);

        const auto window_size{ this->m_window.render_size() };

        auto top_bottom_collision = [&](const component::position& pos) {
            bool top_collision = pos.y - (rect_size.height / 2.0) <= 0.0;
            bool bottom_collision = pos.y + (rect_size.height / 2.0) >= window_size.height;
            return top_collision || bottom_collision;
        };

        auto left_right_collision = [&](const component::position& pos) {
            bool left_collision = pos.x - (rect_size.width / 2.0) <= 0.0;
            bool right_collision = pos.x + (rect_size.width / 2.0) >= window_size.width;
            return left_collision || right_collision;
        };

        m_world.each(
            [&](flecs::entity, component::position& p, component::velocity& v, component::style&) {
                p.x += v.x * delta_time;
                p.y += v.y * delta_time;

                if (left_right_collision(p))
                    v.x = -v.x;
                if (top_bottom_collision(p))
                    v.y = -v.y;
            });
    }

    void Game::render(float)
    {
        m_window.render([&]() {
            raylib::ClearBackground(color::lightgray);

            m_world.each([](const component::position& p, const component::velocity&,
                            const component::style& s) {
                raylib::DrawRectangle(
                    static_cast<int32_t>(p.x) - static_cast<int32_t>(rect_size.width / 2.0),
                    static_cast<int32_t>(p.y) - static_cast<int32_t>(rect_size.height / 2.0),
                    static_cast<int32_t>(rect_size.width), static_cast<int>(rect_size.height),
                    s.color);
            });

            raylib::DrawFPS(10, 10);
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
