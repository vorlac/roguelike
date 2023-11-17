#include "core/game.hpp"
#include "core/options.hpp"

namespace SDL3 {
#include <SDL3/SDL_main.h>
}

int SDL3::main(int argc, char** argv)
{
    int ret = -1;
    if (!rl::parse_args(argc, argv))
        return 1;
    else
    {
        rl::Game game{};
        ret = game.run();
    }

    SDL3::SDL_Quit();
    return ret;
}
