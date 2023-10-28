#include <chrono>
#include <memory>
#include <string>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/application.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/game.hpp"
#include "core/ui/dialog.hpp"
#include "core/ui/gui.hpp"
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
    Game::Game()
        : m_gui{ new ui::GUI }
    {
    }

    Game::~Game()
    {
        if (m_gui != nullptr)
        {
            delete m_gui;
            m_gui = nullptr;
        }
    }

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
        this->framerate(60);
        scene::set_active<scene::main_menu_scene>(m_world);

        if (m_gui != nullptr)
        {
            ui::Dialog* dialog = new ui::Dialog{ 100, 100, 1024, 768, "asdf" };
            dialog->set_type(ui::Control::Type::Dialog);
            m_gui->add_control(dialog);
        }

        while (!this->should_quit()) [[unlikely]]
        {
            m_gui->update_gui(m_input);

            m_window.begin_drawing();
            raylib::ClearBackground(color::darkgray);
            m_gui->draw_gui();
            raylib::DrawRectangle(0, 0, 95, 40, color::black);
            raylib::DrawFPS(10, 10);
            m_window.end_drawing();
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
