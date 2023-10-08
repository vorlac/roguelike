#pragma once

#include <concepts>
#include <type_traits>

namespace rl
{
    template <typename T>
    concept FloatingPoint = std::disjunction_v<std::is_same<T, float>, std::is_same<T, double>>;

    template <typename T>
    concept Integer = std::disjunction_v<std::is_same<T, int32_t>, std::is_same<T, uint32_t>>;

    template <typename T>
    concept Numeric = FloatingPoint<T> || Integer<T>;

    template <auto N>
    concept PositiveNumeric = Numeric<decltype(N)> && N > 0;

    template <auto N>
    concept NegativeNumeric = Numeric<decltype(N)> && N < 0;

    template <auto N>
    concept PositiveFloat = PositiveNumeric<N> && std::is_floating_point_v<decltype(N)>;

    template <auto N>
    concept PositiveInteger = PositiveNumeric<N> && std::is_integral_v<decltype(N)>;

    template <typename T>
    concept BufferElement = std::movable<T>;
}
