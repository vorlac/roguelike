#include "core/application.hpp"
#include "utils/options.hpp"

int main(const int argc, char** argv)
{
    int ret = 0;
    if (!rl::parse_args(argc, argv))
        ret = 1;
    else {
        rl::Application game{};
        ret = game.run() ? 0 : -1;
    }

    return ret;
}
