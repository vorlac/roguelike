#pragma once

#include "math/dimension2d.hpp"
#include "math/point2d.hpp"

namespace rl
{
    template <typename T = float>
        requires Numeric<T>
    struct rect2
    {
        point2<T> pt{ .x = 0, .y = 0 };
        dimension2d<T> dims{ .width = 0, .height = 0 };
    };

    using rect2f = rect2<float>;
    using rect2i = rect2<int32_t>;
    using rect2u = rect2<uint32_t>;
}
