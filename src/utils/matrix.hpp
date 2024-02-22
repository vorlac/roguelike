#pragma once

#include <array>

#include "utils/numeric.hpp"

namespace rl::math {
    template <typename T = f32, u8 X = 3, u8 Y = 3>
    struct matrix
    {
        std::array<T, X * Y> = { 0 };
    };
}
