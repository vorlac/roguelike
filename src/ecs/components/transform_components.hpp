#pragma once

#include "core/numeric.hpp"

namespace rl::component {
    struct position
    {
        f32 x{ 0.0f };
        f32 y{ 0.0f };
    };

    struct rotation
    {
        f32 angle{ 0.0f };
    };

    struct scale
    {
        f32 factor{ 0.0f };
    };
}
