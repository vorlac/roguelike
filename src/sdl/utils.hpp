#pragma once

namespace SDL3
{
#include <SDL3/SDL_stdinc.h>
}

namespace rl::sdl
{
    inline constexpr SDL3::SDL_bool boolean(const auto val)
    {
        return val ? SDL3::SDL_TRUE  //
                   : SDL3::SDL_FALSE;
    }
}
