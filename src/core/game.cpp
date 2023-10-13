#include "core/game.hpp"

#include <fmt/format.h>

#include "core/application.hpp"
#include "core/ecs/components.hpp"
#include "core/ecs/observers.hpp"
#include "core/ecs/scenes.hpp"
#include "core/ecs/systems.hpp"

namespace rl
{
    Game::Game()
        : m_world{}
    {
        this->initialize_scenes();
        this->initialize_systems();
    }

    Game::~Game()
    {
        m_world.quit();
    }

    void Game::initialize_scenes()
    {
        scenes::init_level_scenes(m_world);
    }

    void Game::initialize_systems()
    {
        systems::init_level_systems(m_world);
    }

    bool Game::run()
    {
        m_world.add<scenes::ActiveScene, scenes::MainMenu>();

        while (!this->should_quit())
        {
            this->update();
            this->render();
        }

        return 0;
    }

    void Game::update()
    {
        m_world.progress(this->delta_time());
    }

    bool Game::render()
    {
        static std::string text{};
        text = fmt::format("FPS: {}", this->framerate());

        m_window.render([&] {
            ::ClearBackground(RAYWHITE);
            ::DrawText(text.data(), 190, 200, 20, GRAY);
        });

        return true;
    }

    bool Game::should_quit()
    {
        return m_world.should_quit() || m_window.should_close();
    }
}
