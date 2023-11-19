#pragma once

#include <cmath>
#include <memory>
#include <utility>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/memory.hpp"

namespace rl::ds {
    template <rl::numeric T>
    struct dimensions
    {
        T width{ cast::to<T>(0) };
        T height{ cast::to<T>(0) };

        constexpr inline dimensions()
            : width{ cast::to<T>(0) }
            , height{ cast::to<T>(0) }
        {
        }

        constexpr inline dimensions(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

        constexpr inline dimensions(const dimensions<T>& other)
            : width{ other.width }
            , height{ other.height }
        {
        }

        constexpr inline dimensions(dimensions<T>&& other)
            : width{ std::move(other.width) }
            , height{ std::move(other.height) }
        {
        }

        constexpr static inline dimensions<T> null()
        {
            return {
                cast::to<T>(0.0),
                cast::to<T>(0.0),
            };
        }

        constexpr static inline dimensions<T> zero()
        {
            return {
                cast::to<T>(0.0),
                cast::to<T>(0.0),
            };
        }

        constexpr inline T area() const
            requires rl::floating_point<T>
        {
            const T area = width * height;
            constexpr T epsilon = std::numeric_limits<T>::epsilon();
            if (std::abs(area) <= epsilon)
                return cast::to<T>(0.0);
            return area;
        }

        constexpr inline T area() const
            requires rl::integer<T>
        {
            return width * height;
        }

        constexpr inline dimensions<T>& operator=(const dimensions<T>& other)
        {
            this->height = other.height;
            this->width = other.width;
            return *this;
        }

        constexpr inline dimensions<T>& operator=(dimensions<T>&& other)
        {
            return this->operator=(other);
        }

        constexpr inline bool operator==(const dimensions<T>& other) const
        {
            return 0 == rl::memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        constexpr inline bool operator!=(const dimensions<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr inline dimensions<T>& operator/=(auto div)
        {
            this->width /= cast::to<T>(div);
            this->height /= cast::to<T>(div);
            return *this;
        }

        constexpr inline dimensions<T> operator/(auto div) const
        {
            return {
                this->width / cast::to<T>(div),
                this->height / cast::to<T>(div),
            };
        }

        constexpr inline dimensions<T>& operator*=(auto mul)
        {
            this->width *= cast::to<T>(mul);
            this->height *= cast::to<T>(mul);
            return *this;
        }

        constexpr inline dimensions<T> operator*(auto mul) const
        {
            return dimensions<T>{
                cast::to<T>(this->width * mul),
                cast::to<T>(this->height * mul),
            };
        }
    };
}
