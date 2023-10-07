#pragma once

#include <concepts>
#include <type_traits>

namespace rl
{
    template <auto N>
    concept Number = std::is_integral_v<decltype(N)> || std::is_floating_point_v<decltype(N)>;

    template <auto N>
    concept PositiveNumber = Number<N> && N > 0;

    template <auto N>
    concept NegativeNumber = Number<N> && N < 0;

    template <auto N>
    concept PositiveFloat = PositiveNumber<N> && std::is_floating_point_v<decltype(N)>;

    template <auto N>
    concept PositiveInteger = PositiveNumber<N> && std::is_integra_vl<decltype(N)>;

    template <typename T>
    concept NullType = std::is_same_v<T, nullptr_t> || std::is_null_pointer_v<T>;

    template <typename T>
    concept NonNullType = std::negation_v<std::is_same<T, nullptr_t>>
                          || std::negation_v<std::is_null_pointer<T>>;

    template <typename T>
    concept BufferElement = std::movable<T>;
}
