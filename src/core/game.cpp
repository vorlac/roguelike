#include <chrono>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/application.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/game.hpp"
#include "core/gui.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/benchmark_scene.hpp"
#include "ecs/scenes/main_menu_scene.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "utils/io.hpp"
#include "utils/time.hpp"

namespace rl
{

    bool Game::setup()
    {
        Application::setup();

        scene::main_menu::init(m_world);
        scene::benchmark::init(m_world, this->m_window.render_size());

        // restrict to one active scene at a time
        m_world.component<scene::active>().add(flecs::Exclusive);
        scene::set_active<scene::benchmark_scene>(m_world);

        return true;
    }

#define RL_PROTOTYPING 1
#if (RL_PROTOTYPING)

    bool Game::run()
    {
        this->setup();

        scene::set_active<scene::main_menu_scene>(m_world);

        while (!this->should_quit()) [[unlikely]]
        {
            m_window.render([] {
                return true;
            });
        }

        return true;
    }

#else

    bool Game::run()
    {
        this->setup();

        while (!this->should_quit()) [[unlikely]]
            m_world.progress();

        return true;
    }

#endif

    bool Game::teardown()
    {
        m_world.quit();
        Application::teardown();
        return true;
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
