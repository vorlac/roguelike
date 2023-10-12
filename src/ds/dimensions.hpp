#pragma once

#include "ds/vector2d.hpp"

namespace rl
{
    template <typename T = float>
        requires Numeric<T>
    struct dimensions
    {
        T width{ 0 };
        T height{ 0 };
    };
}
