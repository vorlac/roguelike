#pragma once

#include "utils/numeric_traits.hpp"

namespace rl::math {
    struct Units
    {
        enum Binary : u64 {
            Unknown = 0,
            Bit = 1,
            Byte = 8 * Bit,
            Kilobyte = 1024 * Byte,
            Megabyte = 1024 * Kilobyte,
            Gigabyte = 1024 * Megabyte,
        };
    };

    inline f32 inverse_lerp(const f32 from, const f32 to, const f32 val)
    {
        return (val - from) / (to - from);
    }

    template <rl::integer I>
    constexpr auto to_bytes(I val, const Units::Binary unit_in,
                            const Units::Binary unit_out = Units::Binary::Unknown)
    {
        return (val * std::to_underlying(unit_in)) / std::to_underlying(unit_out);
    }

    template <rl::numeric T>
    constexpr T max(const T& a, const T& b)
    {
        return a > b ? a : b;
    }

    template <rl::numeric T>
    constexpr T min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template <rl::numeric T>
    constexpr T abs(T val)
    {
        return val < 0 ? -val : val;
    }
}
