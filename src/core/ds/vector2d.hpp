#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>

#include "core/utils/concepts.hpp"
#include "thirdparty/raylib.hpp"

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
        vector2(const raylib::Vector2& other)
        {
            std::memcpy(this, &other, sizeof(*this));
        }

        operator raylib::Vector2() const
        {
            return *reinterpret_cast<const raylib::Vector2*>(this);
        }
    };
}
