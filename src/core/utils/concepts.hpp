#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

#include "core/numeric_types.hpp"

namespace rl::inline constraint
{
    template <typename T, typename... TOther>
    concept any_of = (std::same_as<T, TOther> || ...);

    template <typename T>
    concept floating_point = constraint::any_of<T, f32, f64, lf64>;

    template <typename T>
    concept signed_integer = constraint::any_of<T, i8, i16, i32, i64>;

    template <typename T>
    concept unsigned_integer = constraint::any_of<T, u8, u16, u32, u64, size_t>;

    template <typename T>
    concept integer = constraint::unsigned_integer<T> || constraint::signed_integer<T>;

    template <typename T>
    concept numeric = constraint::floating_point<T> || constraint::integer<T>;

    template <auto N>
    concept positive_numeric = constraint::numeric<decltype(N)> && N > 0;

    template <auto N>
    concept negative_numeric = constraint::numeric<decltype(N)> && N < 0;

    template <auto N>
    concept positive_floating_point = constraint::positive_numeric<N> &&
                                      constraint::floating_point<decltype(N)>;

    template <auto N>
    concept negative_floating_point = constraint::negative_numeric<N> &&
                                      constraint::floating_point<decltype(N)>;

    template <auto N>
    concept positive_integer = constraint::positive_numeric<N> && constraint::integer<decltype(N)>;

    template <auto N>
    concept negative_integer = constraint::negative_numeric<N> && constraint::integer<decltype(N)>;

    template <typename L, typename R>
    concept higher_max = std::numeric_limits<L>::max() > std::numeric_limits<R>::max();

    template <typename L, typename R>
    concept lower_min = std::numeric_limits<L>::min() < std::numeric_limits<R>::min();

}
