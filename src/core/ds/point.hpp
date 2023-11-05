#pragma once

#include <type_traits>

#include "core/ds/vector2d.hpp"
#include "core/utils/concepts.hpp"

namespace rl::ds
{
    template <rl::numeric T>
    using point = vector2<T>;
}
