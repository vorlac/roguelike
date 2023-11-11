#pragma once

#include <flecs.h>

#include "core/ds/dimensions.hpp"
#include "sdl/sdl.hpp"

namespace rl {
    class Game
    {
    public:
        bool setup();
        bool run();
        bool teardown();
        bool run_tests(i32 iterations = 25);

        bool process_inputs() const;
        void quit();

        sdl::sdl_app& sdl();

    protected:
        flecs::world m_world{};
        sdl::sdl_app m_sdl{};
    };
}
