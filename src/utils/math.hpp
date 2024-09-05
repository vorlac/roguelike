#pragma once

#include <numeric>

#include "utils/numeric.hpp"
#include "utils/numeric_traits.hpp"

namespace rl::math {
    struct Units {
        enum Binary : u64 {
            Unknown = 0,
            Bit = 1,
            Byte = 8 * Bit,
            Kilobyte = 1024 * Byte,
            Megabyte = 1024 * Kilobyte,
            Gigabyte = 1024 * Megabyte,
        };
    };

    inline f32 inverse_lerp(const f32 from, const f32 to, const f32 val) {
        return (val - from) / (to - from);
    }

    template <rl::integer I>
    constexpr auto to_bytes(I val, const Units::Binary unit_in,
                            const Units::Binary unit_out = Units::Binary::Unknown) {
        return (val * std::to_underlying(unit_in)) / std::to_underlying(unit_out);
    }

    template <rl::numeric T1, rl::numeric T2>
        requires rl::signed_integer<T1, T2> ||
                 rl::unsigned_integer<T1, T2> ||
                 rl::floating_point<T1, T2>
    constexpr auto max(const T1& a, const T2& b) {
        return a > b ? a : b;
    }

    template <rl::numeric T1, rl::numeric T2>
        requires rl::signed_integer<T1, T2> ||
                 rl::unsigned_integer<T1, T2> ||
                 rl::floating_point<T1, T2>
    constexpr auto min(const T1& a, const T2& b) {
        return a < b ? a : b;
    }

    template <rl::numeric T>
    constexpr T abs(T val) {
        return val < 0 ? -val : val;
    }

    template <rl::floating_point F>
    consteval auto floor(F val) {
        if (val > 0)
            return static_cast<i32>(val);
        return static_cast<i32>(val - 1);
    }

    template <rl::floating_point F>
    consteval auto ceil(F val) {
        if (equal<F, F>(val, 0))
            return 0;
        if (val > 0)
            return static_cast<i32>(val + 1);
        return static_cast<i32>(val);
    }

    template <rl::integer I>
    consteval auto pow(I num, u8 exp) {
        auto ret{ 0 };
        if (num != 0) {
            ret = 1;
            while (exp != 0) {
                if ((exp & 0x0001) != 0)
                    ret *= num;

                exp /= 2;
                num *= num;
            }
        }
        return ret;
    }
}
