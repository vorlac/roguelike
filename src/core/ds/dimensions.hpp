#pragma once

#include <memory>
#include <utility>

#include "core/utils/concepts.hpp"
#include "core/utils/conversions.hpp"
#include "core/utils/memory.hpp"

namespace rl::ds {
    template <rl::numeric T>
    struct dimensions
    {
        T width{ cast::to<T>(0) };
        T height{ cast::to<T>(0) };

        constexpr dimensions()
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
            : width{ std::move(other.width) }
            , height{ std::move(other.height) }
        {
        }

        constexpr static inline dimensions<T> null()
        {
            return {
                cast::to<T>(0),
                cast::to<T>(0),
            };
        }

        constexpr static inline dimensions<T> zero()
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

        constexpr inline dimensions& operator=(const dimensions<T>& other)
        {
            this->height = other.height;
            this->width = other.width;
            return *this;
        }

        constexpr inline dimensions& operator=(dimensions<T>&& other)
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
