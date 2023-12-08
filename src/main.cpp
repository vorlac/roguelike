#include "core/game.hpp"
#include "sdl/defs.hpp"
#include "utils/options.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_main.h>
SDL_C_LIB_END

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
