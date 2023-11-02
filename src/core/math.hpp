#pragma once

#include "core/numeric_types.hpp"

namespace rl::math
{
    inline f32 inverse_lerp(f32 from, f32 to, f32 val)
    {
        return (val - from) / (to - from);
    }
}