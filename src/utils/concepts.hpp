#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

#include "utils/numeric.hpp"

namespace rl {
    template <typename T>
    concept refcountable = requires(const T& t) {
        t.acquire_ref();
        t.release_ref();
    };

    template <typename T, typename... TOther>
    concept any_of = (std::same_as<T, TOther> || ...);

    template <typename T>
    concept floating_point = any_of<T, f32, f64, lf64>;

    template <typename T>
    concept signed_integer = any_of<T, i8, i16, i32, i64>;

    template <typename T>
    concept unsigned_integer = any_of<T, u8, u16, u32, u64, size_t>;

    template <typename T>
    concept integer = (unsigned_integer<T> || signed_integer<T>);

    template <typename T>
    concept numeric = (floating_point<T> || integer<T>);

    template <auto N>
    concept positive_numeric = (numeric<decltype(N)> && N > 0);

    template <auto N>
    concept negative_numeric = (numeric<decltype(N)> && N < 0);

    template <auto N>
    concept positive_floating_point = positive_numeric<N> && floating_point<decltype(N)>;

    template <auto N>
    concept negative_floating_point = negative_numeric<N> && floating_point<decltype(N)>;

    template <auto N>
    concept positive_integer = positive_numeric<N> && integer<decltype(N)>;

    template <auto N>
    concept negative_integer = negative_numeric<N> && integer<decltype(N)>;

    template <typename L, typename R>
    concept higher_max = std::numeric_limits<L>::max() > std::numeric_limits<R>::max();

    template <typename L, typename R>
    concept lower_min = std::numeric_limits<L>::min() < std::numeric_limits<R>::min();

    template <typename T>
    concept scoped_enum = std::is_scoped_enum_v<T>;
}
