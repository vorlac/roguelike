#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "core/application.hpp"
#include "core/ui/crtp/crtp_label.hpp"
#include "core/ui/crtp/crtp_widget.hpp"
#include "graphics/vg/nanovg.hpp"

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
