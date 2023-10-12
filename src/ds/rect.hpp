#pragma once

#include "ds/dimensions.hpp"
#include "ds/point.hpp"

namespace rl
{
    template <typename T = float>
        requires Numeric<T>
    struct rect
    {
        point<T> pt{ .x = 0, .y = 0 };
        dimensions<T> dims{ .width = 0, .height = 0 };

        rect(rect&& other)
        {
            this.pt = std::move(other.pt);
            this.dims = std::move(other.dims);
        }
    };
}
