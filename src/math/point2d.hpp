#pragma once

#include "math/vector2d.hpp"

namespace rl
{
    template <typename T>
    using point2 = vector2<T>;
    using point2f = point2<float>;
    using point2i = point2<int32_t>;
    using point2u = point2<uint32_t>;
}
