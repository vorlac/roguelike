#pragma once

#include <limits>
#include <numbers>
#include <type_traits>

#include "utils/concepts.hpp"

namespace rl::traits {

    template <rl::numeric T>
    struct numeric_traits
    {
        using type = T;
        using limits = std::numeric_limits<T>;

        constexpr static inline T min{ std::numeric_limits<T>::min() };
        constexpr static inline T max{ std::numeric_limits<T>::max() };

        constexpr static inline bool is_flt{ std::is_floating_point_v<T> };
        constexpr static inline bool is_int{ std::is_integral_v<T> };

        static_assert(is_flt != is_int);
    };

    template <rl::integer T>
    struct integer_traits : numeric_traits<T>
    {
        constexpr static inline bool is_signed{ std::is_signed_v<T> };
    };

    template <rl::floating_point T>
    struct float_traits : numeric_traits<T>
    {
        constexpr static inline T eps{ std::numeric_limits<T>::epsilon() };
        constexpr static inline T pi{ std::numbers::pi_v<T> };
    };
}
