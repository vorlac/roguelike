#pragma once

#include <concepts>
#include <cstdint>
#include <raylib.h>
#include <type_traits>

#include "utils/concepts.hpp"

namespace rl
{
    template <typename T = float>
        requires Numeric<T>
    struct vector2
    {
        T x{ 0 };
        T y{ 0 };

        vector2(Vector2&& vec2)
        {
            *this = std::move(vec2);
        }

        vector2(vector2&& other)
        {
            *this = std::move(other);
        }

        vector2 operator=(const vector2& other)
        {
            memcpy(this, &other, sizeof(*this));
        }

        vector2 operator=(const Vector2& other)
        {
            memcpy(this, &other, sizeof(*this));
        }

        operator Vector2()
            requires FloatingPoint<T>
        {
            return { x, y };
        }
    };
}
