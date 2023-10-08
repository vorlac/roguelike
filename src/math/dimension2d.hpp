#pragma once

#include "math/vector2d.hpp"

namespace rl
{
    template <typename T = float>
        requires Numeric<T>
    struct dimension2d
    {
        T width{ 0 };
        T height{ 0 };
    };

    using dime2f = dimension2d<float>;
    using dims2i = dimension2d<int32_t>;
    using dims2u = dimension2d<uint32_t>;
}
