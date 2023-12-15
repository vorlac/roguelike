#include "core/application.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_main.h>
SDL_C_LIB_END

// template <typename... T>
//     requires(std::same_as<T, int32_t> && ...)
// constexpr static inline int sum(T... args)
//{
//     return {... + args };
// }
//
// int32_t a = sum(1, 2, 3, 4);
// int32_t b = sum(1, 2);

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
