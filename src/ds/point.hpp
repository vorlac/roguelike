#pragma once

#include "ds/vector2d.hpp"

namespace rl
{
    template <typename T>
    using point = vector2<T>;

    template <typename T>
    using position = point<T>;
}
