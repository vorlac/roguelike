#pragma once

#include "sdl/color.hpp"

namespace rl::component {
    struct style
    {
        sdl::Color<u8> color{ 128, 255, 0, 255 };
    };
}
