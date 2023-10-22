#pragma once

#include <flecs.h>

#include "core/application.hpp"
#include "core/ds/dimensions.hpp"
#include "core/input/input.hpp"

namespace rl::ui
{
    class GUI;
}

namespace rl
{
    class Game : public Application
    {
    public:
        Game();
        ~Game();

        bool setup();
        bool run();
        bool teardown();

        bool should_quit() const;
        void quit() const;

    protected:
        flecs::world m_world{};
        ui::GUI* m_gui{ nullptr };
    };
}
