#pragma once

#include "utils/concepts.hpp"

#include <concepts>
#include <cstdint>
#include <raylib.h>
#include <type_traits>

namespace rl
{
    template <typename T = float>
        requires Numeric<T>
    struct vector2
    {
        T x{ 0 };
        T y{ 0 };
    };

    using vector2f = vector2<float>;
    using vector2i = vector2<int32_t>;
    using vector2u = vector2<uint32_t>;

}
