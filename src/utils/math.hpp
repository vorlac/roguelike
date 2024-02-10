#pragma once
#include <limits>
#include <type_traits>

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

    namespace detail {
        template <rl::numeric A, rl::numeric B>
            requires std::same_as<A, B>
        constexpr auto max(const A a, const B b) -> decltype(a > b ? a : b)
        {
            return a > b ? a : b;
        }

        template <rl::numeric A, rl::numeric B>
        constexpr auto min(const A a, const B b) -> decltype(a < b ? a : b)
            requires std::same_as<A, B>
        {
            return a < b ? a : b;
        }
    }

    template <rl::numeric T>
    constexpr T abs(T val)
    {
        return val < 0 ? -val : val;
    }

    template <rl::floating_point A, rl::floating_point B>
    constexpr auto is_equal(A a, B b) -> bool
    {
        if constexpr (rl::lower_precision<A, B>)
        {
            using lp_float_t = traits::float_traits<A>;
            return math::abs(a - b) <= lp_float_t::eps * math::abs(a + b) ||
                   math::abs(a - b) < lp_float_t::min;
        }
        else
        {
            using lp_float_t = traits::float_traits<B>;
            return math::abs(a - b) <= lp_float_t::eps * math::abs(a + b) ||
                   math::abs(a - b) < lp_float_t::min;
        }
    }

    template <rl::integer A, rl::integer B>
    constexpr bool is_equal(A a, B b)
    {
        return a == b;
    }
}
