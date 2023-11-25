#pragma once

#include "core/numeric_types.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"

namespace rl::math {
    inline f32 inverse_lerp(f32 from, f32 to, f32 val)
    {
        return (val - from) / (to - from);
    }

    template <rl::numeric TOut, rl::numeric TIn>
    TOut clamp(TIn v, TOut low, TOut high)
    {
        return rl::cast::to<TOut>(v < low ? low : v > high ? high : v);
    }
}
