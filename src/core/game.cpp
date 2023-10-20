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
        scene::init_demo_scene(m_world, std::move(this->m_window.render_size()));
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

        while (!this->should_quit()) [[unlikely]]
        {
            raylib::BeginDrawing();
            raylib::ClearBackground(color::lightgray);

            m_world.progress();

            raylib::DrawRectangle(0, 0, 95, 40, color::black);
            raylib::DrawFPS(10, 10);
            raylib::EndDrawing();
        }

        return 0;
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
