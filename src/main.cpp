#include <argparse/argparse.hpp>

#include "core/game.hpp"
#include "core/options.hpp"

#include "thirdparty/raygui.hpp"

int main(int argc, char** argv)
{
    if (!rl::parse_args(argc, argv))
        return -1;

    rl::Game game{};
    return game.run();
}
