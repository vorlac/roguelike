#include "core/application.hpp"

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
        rl::Application game{};
        ret = game.run();
    }

    SDL3::SDL_Quit();
    return ret;
}
