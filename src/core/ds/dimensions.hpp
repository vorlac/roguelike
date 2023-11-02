#pragma once

#include "core/ds/vector2d.hpp"

namespace rl::ds
{
    template <typename T>
        requires Numeric<T>
    struct dimensions
    {
        T width{ 0 };
        T height{ 0 };

        constexpr dimensions(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

        constexpr dimensions()
        {
        }

        constexpr ~dimensions()
        {
        }

        constexpr auto area() const -> decltype(width * height)
        {
            return width * height;
        }
    };
}
