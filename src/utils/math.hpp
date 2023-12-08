#pragma once

#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

namespace rl::math {
    struct units
    {
        enum binary : u64 {
            unknown = 0,
            bit = 1,
            byte = 8 * bit,
            kilobyte = 1024 * byte,
            megabyte = 1024 * kilobyte,
            gigabyte = 1024 * megabyte,
        };
    };

    inline f32 inverse_lerp(f32 from, f32 to, f32 val)
    {
        return (val - from) / (to - from);
    }

    template <rl::integer I>
    constexpr inline auto to_bytes(I val, units::binary unit_in,
                                   units::binary unit_out = units::binary::unknown)
    {
        return (val * std::to_underlying(unit_in)) / std::to_underlying(unit_out);
    }
}
