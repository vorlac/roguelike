#pragma once

#include "ds/color.hpp"

namespace rl::component {
    struct style
    {
        ds::color<u8> color{ 128, 255, 0, 255 };
    };
}
