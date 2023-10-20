#pragma once

#include "core/ds/vector2d.hpp"

namespace rl::ds
{
    template <typename T = float>
    using point = vector2<T>;

    template <typename T = float>
    using position = point<T>;

    template <typename T = float>
    using velocity = point<T>;
}
