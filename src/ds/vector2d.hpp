#pragma once

#include <concepts>
#include <cstdint>
#include <raylib.h>
#include <type_traits>

#include "utils/concepts.hpp"

namespace rl::ds
{
    template <typename T = float>
        requires Numeric<T>
    struct vector2
    {
        T x{ 0 };
        T y{ 0 };

        vector2(::Vector2 other);
        operator Vector2();
    };

    template <typename T>
        requires Numeric<T>
    vector2<T>::vector2(::Vector2 other)
    {
        x = other.x;
        y = other.y;
    }

    template <typename T>
        requires Numeric<T>
    vector2<T>::operator std::type_identity_t<::Vector2>()
    {
        return *this;
    }
}
