#pragma once

#include <type_traits>

#include "primitives/vector2d.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
    template <rl::numeric T>
    using point = vector2<T>;
}
