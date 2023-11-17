#pragma once

#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_stdinc.h>
SDL_C_LIB_END

namespace rl::sdl {
    constexpr inline SDL3::SDL_bool boolean(const auto val)
    {
        return val ? SDL_TRUE  //
                   : SDL_FALSE;
    }
}
