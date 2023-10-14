#pragma once

#include <concepts>
#include <cstdint>
#include <cstdio>
#include <raylib.h>
#include <type_traits>

#include "ecs/components.hpp"
#include "utils/concepts.hpp"

namespace rl::ds
{
    template <typename T = float>
        requires Numeric<T>
    struct vector2
    {
        T x{ 0 };
        T y{ 0 };

        ~vector2()
        {
        }

        vector2(const T _x, const T _y)
            : x{ _x }
            , y{ _y }
        {
        }

        template <typename V>
            requires std::same_as<T, V>
        vector2(const ::Vector2& other)
        {
            std::memcpy(this, &other, sizeof(*this));
        }

        operator ::Vector2() const
        {
            return *reinterpret_cast<const ::Vector2*>(this);
        }
    };
}
