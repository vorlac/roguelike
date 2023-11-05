#pragma once

#include <flecs.h>

#include "core/ds/dimensions.hpp"
#include "sdl/sdl.hpp"

namespace rl
{
    class Game
    {
    public:
        bool setup();
        bool run();
        bool teardown();

        bool print_log_msg(const char* event_type, const char* text);
        bool process_inputs() const;
        void quit();

    protected:
        flecs::world m_world{};

        sdl::sdl_app m_sdl{};
    };
}
