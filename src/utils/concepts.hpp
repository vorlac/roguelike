#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

#include "utils/numeric.hpp"

namespace rl::inline constraint {
    template <typename T>
    concept refcountable = requires(const T& t) {
        t.acquire_ref();
        t.release_ref();
    };

    template <typename T, typename... TOther>
    concept any_of = (std::same_as<std::type_identity_t<T>, TOther> || ...);

    template <typename T, typename... TOther>
    concept all_of = (std::same_as<std::type_identity_t<T>, TOther> && ...);

    template <typename... T>
    concept floating_point = (any_of<std::type_identity_t<T>, f32, f64> && ...);

    template <typename... T>
    concept signed_integer = (any_of<std::type_identity_t<T>, i8, i16, i32, i64> && ...);

    template <typename... T>
    concept unsigned_integer = (any_of<std::type_identity_t<T>, u8, u16, u32, u64> && ...);

    template <typename... T>
    concept integer = ((unsigned_integer<std::type_identity_t<T>> || signed_integer<std::type_identity_t<T>>) && ...);

    template <typename... T>
    concept numeric = ((floating_point<std::type_identity_t<T>> || integer<std::type_identity_t<T>>) && ...);

    template <auto... N>
    concept positive_numeric_v = ((numeric<decltype(N)> && N > 0) && ...);

    template <auto... N>
    concept negative_numeric_v = ((numeric<decltype(N)> && N < 0) && ...);

    template <auto A, auto B>
    concept equal_v = std::same_as<decltype(A), decltype(B)> &&
                      numeric<decltype(A), decltype(B)> && A == B;

    template <auto A, auto B>
    concept greater_than = std::same_as<decltype(A), decltype(B)> &&
                           numeric<decltype(A), decltype(B)> && A > B;

    template <auto A, auto B>
    concept less_than = std::same_as<decltype(A), decltype(B)> &&
                        numeric<decltype(A), decltype(B)> && A < B;

    template <auto A, auto B>
    concept greater_than_or_eq = (greater_than<A, B> || equal_v<A, B>);

    template <auto A, auto B>
    concept less_than_or_eq = (greater_than<A, B> || equal_v<A, B>);

    template <auto... N>
    concept positive_floating_point = ((positive_numeric_v<N> && floating_point<decltype(N)>) || ...);

    template <auto... N>
    concept negative_floating_point = ((negative_numeric_v<N> && floating_point<decltype(N)>) || ...);

    template <auto... N>
    concept positive_integer = ((positive_numeric_v<N> && integer<decltype(N)>) || ...);

    template <auto... N>
    concept negative_integer = ((negative_numeric_v<N> && integer<decltype(N)>) || ...);

    template <typename L, typename R>
    concept higher_precision = floating_point<L, R> && greater_than<std::numeric_limits<L>::max(), std::numeric_limits<R>::max()>;
    template <typename L, typename R>
    concept lower_precision = floating_point<L, R> &&
                              less_than_or_eq<std::numeric_limits<L>::max(),
                                              std::numeric_limits<R>::max()>;
    template <typename L, typename R>
    concept lower_min = integer<L, R> &&
                        std::numeric_limits<L>::min() < std::numeric_limits<R>::min();

    template <typename L, typename R>
    concept higher_min = integer<L, R> &&
                         std::numeric_limits<L>::min() > std::numeric_limits<R>::min();

    template <typename L, typename R>
    concept lower_max = integer<L, R> &&
                        std::numeric_limits<L>::max() < std::numeric_limits<R>::max();

    template <typename L, typename R>
    concept higher_max = integer<L, R> &&
                         std::numeric_limits<L>::max() > std::numeric_limits<R>::max();

    template <typename T>
    concept scoped_enum = std::is_scoped_enum_v<T>;
}
