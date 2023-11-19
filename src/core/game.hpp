#pragma once

#include <flecs.h>

#include "ds/dimensions.hpp"
#include "sdl/application.hpp"

namespace rl {
    class Game
    {
    public:
        bool setup();
        bool run();
        bool teardown();
        bool run_tests(i32 iterations = 25);
        bool quit_requested() const;
        bool handle_events();
        void quit();

        sdl::Application& sdl();

    protected:
        flecs::world m_world{};
        sdl::Application m_sdl{};
    };
}
