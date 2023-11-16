#pragma once

namespace SDL3 {
#include <SDL3/SDL_stdinc.h>
}

namespace rl::sdl {
    constexpr inline SDL3::SDL_bool boolean(const auto val)
    {
        return val ? SDL_TRUE  //
                   : SDL_FALSE;
    }
}
