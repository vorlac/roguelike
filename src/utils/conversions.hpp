#pragma once

#include <concepts>
#include <limits>
#include <type_traits>
#include <typeinfo>

#include "core/assert.hpp"
#include "utils/concepts.hpp"
#include "utils/math.hpp"
#include "utils/numeric.hpp"
#include "utils/numeric_traits.hpp"

namespace rl::math {

    template <rl::floating_point A, rl::floating_point B>
    constexpr bool equal(const A& lhs, const B& rhs) {
        if constexpr (rl::lower_precision<A, B>) {
            using lp_float_t = rl::traits::float_traits<A>;
            return math::abs(lhs - rhs) <= lp_float_t::eps * math::abs(lhs + rhs) ||
                   math::abs(lhs - rhs) < lp_float_t::min;
        }
        else {
            using lp_float_t = rl::traits::float_traits<B>;
            return math::abs(lhs - rhs) <= lp_float_t::eps * math::abs(lhs + rhs) ||
                   math::abs(lhs - rhs) < lp_float_t::min;
        }
    }

    template <rl::integer A, rl::integer B>
    constexpr bool equal(const A lhs, const B rhs) {
        if constexpr (std::is_signed_v<A> && !std::is_signed_v<B>)
            return lhs == static_cast<std::make_signed_t<B>>(rhs);
        if constexpr (std::is_signed_v<B> && !std::is_signed_v<A>)
            return static_cast<std::make_signed_t<B>>(lhs) == rhs;
        else
            return lhs == rhs;
    }

    template <rl::numeric T>
    constexpr bool not_equal(const T lhs, const T rhs) {
        return !equal(lhs, rhs);
    }
}

namespace rl {
    template <rl::scoped_enum TEnum>
    constexpr TEnum operator|(const TEnum lhs, const TEnum rhs) {
        return static_cast<TEnum>(std::to_underlying(lhs) | std::to_underlying(rhs));
    }

    template <rl::scoped_enum TEnum, rl::integer TUnderlying>
    constexpr bool operator|(const TUnderlying lhs, const TEnum rhs) {
        return lhs | std::to_underlying(rhs);
    }

    template <rl::scoped_enum TEnum>
    constexpr TEnum& operator|=(TEnum& lhs, const TEnum rhs) {
        lhs = (lhs | rhs);
        return lhs;
    }

    template <rl::scoped_enum TEnum>
    constexpr TEnum operator~(const TEnum val) {
        return static_cast<TEnum>(~std::to_underlying(val));
    }

    template <rl::scoped_enum TEnum>
    constexpr TEnum operator&(const TEnum lhs, const TEnum rhs) {
        return static_cast<TEnum>(std::to_underlying(lhs) & std::to_underlying(rhs));
    }

    template <rl::scoped_enum TEnum, rl::integer TUnderlying>
    constexpr bool operator&(const TUnderlying lhs, const TEnum rhs) {
        return lhs & std::to_underlying(rhs);
    }

    template <rl::scoped_enum TEnum>
    constexpr TEnum operator&=(TEnum& lhs, const TEnum rhs) {
        lhs = static_cast<TEnum>(std::to_underlying(lhs) & std::to_underlying(rhs));
        return lhs;
    }

    template <rl::scoped_enum TEnum, rl::integer TUnderlying>
    constexpr bool operator==(const TUnderlying lhs, const TEnum rhs) {
        return static_cast<std::underlying_type_t<TEnum>>(lhs) == std::to_underlying(rhs);
    }

    template <rl::scoped_enum TEnum, rl::integer TUnderlying>
    constexpr bool operator==(const TEnum lhs, const TUnderlying rhs) {
        return std::to_underlying(lhs) == static_cast<std::underlying_type_t<TEnum>>(rhs);
    }

    template <rl::scoped_enum TEnum, rl::integer TUnderlying>
    constexpr bool operator!=(const TUnderlying lhs, const TEnum rhs) {
        return !(lhs == rhs);
    }

    template <rl::scoped_enum TEnum, rl::integer TUnderlying>
    constexpr bool operator!=(const TEnum lhs, const TUnderlying rhs) {
        return !(lhs == rhs);
    }
};

namespace rl::inline cast {
    template <typename To, typename From>
    struct within_bounds {
        constexpr static bool value(From val)
            requires rl::signed_integer<To> && rl::unsigned_integer<From>
        {
            return static_cast<i64>(val) <= std::numeric_limits<To>::max() &&
                   static_cast<i64>(val) >= std::numeric_limits<To>::min();
        }

        constexpr static bool value(From val)
            requires rl::unsigned_integer<To> && rl::signed_integer<From>
        {
            return static_cast<u64>(val) <= std::numeric_limits<To>::max() &&
                   static_cast<u64>(val) >= std::numeric_limits<To>::min();
        }

        constexpr static bool value(From val) {
            return val <= std::numeric_limits<To>::max() &&
                   val >= std::numeric_limits<To>::lowest();
        }
    };

    // widening  conversions
    template <rl::numeric To, rl::numeric From>
    constexpr To to(const From val) {
        return static_cast<To>(val);
    }

    // floating point to integer conversion
    template <rl::integer To, rl::floating_point From>
    constexpr To to(const From val) {
        return static_cast<To>(val >= static_cast<From>(0.0)
                                   ? val + static_cast<From>(0.5)
                                   : val - static_cast<From>(0.5));
    }

    // narrowing integer conversion
    template <rl::integer To, rl::integer From>
        requires(!rl::higher_max<To, From> || !rl::lower_min<To, From>)
    constexpr To to(const From in) {
        const bool value_in_bounds{ within_bounds<To, From>::value(in) };
        debug_assert(value_in_bounds,
                     "narrowing integer numeric cast results in overflow {}  ({}) -> {}",
                     typeid(From).name(), in, typeid(To).name());

        return static_cast<To>(in);
    }

    // narrowing floating point conversion
    template <rl::floating_point To, rl::floating_point From>
        requires(!rl::higher_max<To, From> || !rl::lower_min<To, From>)
    constexpr To to(const From in) {
        const bool value_in_bounds{ within_bounds<To, From>::value(in) };
        debug_assert(value_in_bounds,
                     "narrowing integer numeric cast results in overflow {}  ({}) -> {}",
                     typeid(From).name(), in, typeid(To).name());

        return static_cast<To>(in);
    }
}
