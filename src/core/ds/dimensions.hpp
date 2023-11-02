#pragma once

#include <imgui.h>

#include "core/ds/vector2d.hpp"
#include "core/utils/conversions.hpp"

namespace rl::ds
{
    template <rl::numeric T>
    struct dimensions
    {
        T width{ 0 };
        T height{ 0 };

        constexpr dimensions(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

        constexpr operator ::ImVec2()
            requires std::same_as<T, f32>
        {
            return *cast::to<::ImVec2*>(this);
        }

        constexpr operator ::ImVec2()
            requires(!std::same_as<T, f32>)
        {
            return ::ImVec2{
                cast::to<f32>(this->width),
                cast::to<f32>(this->height),
            };
        }

        constexpr auto area() const -> decltype(width * height)
        {
            return width * height;
        }
    };
}
