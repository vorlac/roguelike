#pragma once

#include <flecs.h>

#include "core/application.hpp"

namespace rl
{
    class Game : public Application
    {
    public:
        Game();
        ~Game();

        bool run();

        void update();
        bool render();

        void initialize_systems();
        void initialize_scenes();

        bool should_quit();

    protected:
        flecs::world m_world{};
    };
}
