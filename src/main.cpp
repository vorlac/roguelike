#include "core/application.hpp"
#include "sdl/defs.hpp"
#include "utils/options.hpp"

int wmain(int argc, wchar_t* argv[], wchar_t* envp)
{
    int ret = -1;
    if (!rl::parse_args(argc, argv))
        ret = 1;
    else
    {
        rl::Application game{};
        ret = game.run();
    }

    SDL3::SDL_Quit();
    return ret;
}
