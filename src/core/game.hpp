#pragma once

#include <flecs.h>

#include "core/application.hpp"
#include "core/ds/dimensions.hpp"

namespace rl
{
    class Game : public Application
    {
    public:
        bool setup();
        bool run();
        bool teardown();

        bool should_quit() const;
        void quit() const;

    protected:
        flecs::world m_world{};
    };
}
