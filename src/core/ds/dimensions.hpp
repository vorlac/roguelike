#pragma once

#include "core/ds/vector2d.hpp"

namespace rl::ds
{
    template <typename T = float>
        requires Numeric<T>
    struct dimensions
    {
        T width{ 0 };
        T height{ 0 };
    };
}