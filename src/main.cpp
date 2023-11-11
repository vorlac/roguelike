#include <fmt/format.h>

#include "core/game.hpp"
#include "core/options.hpp"

namespace SDL3 {
#include <SDL3/SDL_main.h>
}

int SDL3::main(int argc, char** argv)
{
    if (!rl::parse_args(argc, argv))
        return -1;

    rl::Game game{};
    return game.run();
}
