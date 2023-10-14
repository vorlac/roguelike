#pragma once

#include <flecs.h>

#include "core/application.hpp"

namespace rl
{
    class Game : public Application
    {
    public:
        Game() = default;
        ~Game();

        bool run();

        void update(float delta_time);
        void render(float delta_time);

        bool should_quit();

    protected:
        flecs::world m_world{};
    };
}
