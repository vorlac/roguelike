#pragma once

#include "core/ds/vector2d.hpp"
#include "core/utils/concepts.hpp"

namespace rl::ds
{
    template <rl::numeric T>
    using point = std::type_identity<vector2<T>>::type;
}
