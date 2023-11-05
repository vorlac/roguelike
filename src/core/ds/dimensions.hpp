#pragma once

// #include <imgui.h>
#include <memory>
#include <utility>

#include <memory.h>

#include "core/utils/conversions.hpp"

namespace rl::ds
{
    template <rl::numeric T>
    struct dimensions
    {
        T width{ cast::to<T>(0) };
        T height{ cast::to<T>(0) };

        inline constexpr dimensions()
            : width{ cast::to<T>(0) }
            , height{ cast::to<T>(0) }
        {
        }

        constexpr dimensions(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

        constexpr dimensions(const dimensions<T>& other)
            : width{ other.width }
            , height{ other.height }
        {
        }

        constexpr dimensions(dimensions<T>&& other)
            : width{ std::forward<T>(other.width) }
            , height{ std::forward<T>(other.height) }
        {
        }

        constexpr dimensions(dimensions<T>& other)
            : width{ other.width }
            , height{ other.height }
        {
        }

        static constexpr inline dimensions<T> null()
        {
            return {
                cast::to<T>(0),
                cast::to<T>(0),
            };
        }

        static constexpr inline dimensions<T> zero()
        {
            return {
                cast::to<T>(0),
                cast::to<T>(0),
            };
        }

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
