#pragma once

#include <concepts>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/concepts.hpp"

namespace rl::inline cast
{
    template <typename To, typename From>
    struct within_bounds
    {
        static constexpr inline bool value(From val)
            requires rl::signed_integer<To> && rl::unsigned_integer<From>
        {
            return static_cast<i64>(val) <= std::numeric_limits<To>::max() &&
                   static_cast<i64>(val) >= std::numeric_limits<To>::min();
        }

        static constexpr inline bool value(From val)
            requires rl::unsigned_integer<To> && rl::signed_integer<From>
        {
            return static_cast<u64>(val) <= std::numeric_limits<To>::max() &&
                   static_cast<u64>(val) >= std::numeric_limits<To>::min();
        }

        static constexpr inline bool value(From val)
        {
            return val <= std::numeric_limits<To>::max() &&  //
                   val >= std::numeric_limits<To>::lowest();
        }
    };

    // widening  conversions
    template <rl::numeric To, rl::numeric From>
    constexpr inline To to(const From val)
    {
        return static_cast<To>(val);
    }

    // floating point to integer conversion
    template <rl::integer To, rl::floating_point From>
    constexpr inline To to(const From val)
    {
        return static_cast<To>(val >= static_cast<From>(0.0)       //
                                   ? val + static_cast<From>(0.5)  //
                                   : val - static_cast<From>(0.5));
    }

    // narrowing integer conversion
    template <rl::integer To, rl::integer From>
        requires(!rl::higher_max<To, From> || !rl::lower_min<To, From>)
    constexpr inline To to(const From in)
    {
        const bool value_in_bounds{ within_bounds<To, From>::value(in) };
        runtime_assert(value_in_bounds,
                       "narrowing integer numeric cast results in overflow"
                           << typeid(From).name() << "(" << in << ") -> " << typeid(To).name());

        return static_cast<To>(in);
    }

    // narrowing floating point conversion
    template <rl::floating_point To, rl::floating_point From>
        requires(!rl::higher_max<To, From> || !rl::lower_min<To, From>)
    constexpr inline To to(const From in)
    {
        const bool value_in_bounds{ within_bounds<To, From>::value(in) };
        runtime_assert(value_in_bounds,
                       "narrowing floating point cast results in overflow: "
                           << typeid(From).name() << "(" << in << ") -> " << typeid(To).name());

        return static_cast<To>(in);
    }

}
