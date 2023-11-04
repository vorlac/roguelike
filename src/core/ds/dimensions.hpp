#pragma once

// #include <imgui.h>

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

        // constexpr dimensions(const ImVec2& other)
        //     : width{ cast::to<T>(other.x) }
        //     , height{ cast::to<T>(other.y) }
        //{
        // }

        // constexpr operator ::ImVec2()
        //     requires std::same_as<T, f32>
        //{
        //     return *reinterpret_cast<::ImVec2*>(this);
        // }

        // constexpr operator ::ImVec2()
        //     requires(!std::same_as<T, f32>)
        //{
        //     return ::ImVec2{
        //         cast::to<f32>(this->width),
        //         cast::to<f32>(this->height),
        //     };
        // }

        constexpr auto area() const -> decltype(width * height)
        {
            return width * height;
        }

        constexpr dimensions<T> operator/=(auto div)
        {
            this->width /= cast::to<T>(div);
            this->height /= cast::to<T>(div);
            return *this;
        }

        constexpr dimensions<T>&& operator/(auto div)
        {
            return {
                this->width / cast::to<T>(div),
                this->height / cast::to<T>(div),
            };
        }

        constexpr dimensions<T> operator*=(auto mul)
        {
            this->width *= cast::to<T>(mul);
            this->height *= cast::to<T>(mul);
            return *this;
        }

        constexpr dimensions<T>&& operator*(auto mul)
        {
            return {
                this->width * cast::to<T>(mul),
                this->height * cast::to<T>(mul),
            };
        }
    };
}
