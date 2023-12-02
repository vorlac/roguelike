#pragma once

#include <cmath>
#include <memory>
#include <utility>

#include <fmt/format.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/memory.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct dims
    {
        T width{ cast::to<T>(0) };
        T height{ cast::to<T>(0) };

        constexpr inline dims()
            : width{ cast::to<T>(0) }
            , height{ cast::to<T>(0) }
        {
        }

        constexpr inline dims(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

        constexpr inline dims(const dims<T>& other)
            : width{ other.width }
            , height{ other.height }
        {
        }

        constexpr inline dims(dims<T>&& other) noexcept
            : width{ std::move(other.width) }
            , height{ std::move(other.height) }
        {
        }

        constexpr static inline dims<T> null()
        {
            return {
                cast::to<T>(0.0),
                cast::to<T>(0.0),
            };
        }

        constexpr static inline dims<T> zero()
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

        constexpr inline dims<T>& operator=(const dims<T>& other)
        {
            this->height = other.height;
            this->width = other.width;
            return *this;
        }

        constexpr inline dims<T>& operator=(dims<T>&& other) noexcept
        {
            return this->operator=(other);
        }

        constexpr inline bool operator==(const dims<T>& other) const
        {
            return 0 == rl::memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        constexpr inline bool operator!=(const dims<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr inline dims<T>& operator/=(auto div)
        {
            this->width /= cast::to<T>(div);
            this->height /= cast::to<T>(div);
            return *this;
        }

        constexpr inline dims<T> operator/(auto div) const
        {
            return {
                this->width / cast::to<T>(div),
                this->height / cast::to<T>(div),
            };
        }

        constexpr inline dims<T>& operator*=(auto mul)
        {
            this->width *= cast::to<T>(mul);
            this->height *= cast::to<T>(mul);
            return *this;
        }

        constexpr inline dims<T> operator*(auto mul) const
        {
            return dims<T>{
                cast::to<T>(this->width * mul),
                cast::to<T>(this->height * mul),
            };
        }
    };

    template <rl::numeric T>
    constexpr auto format_as(const dims<T>& size)
    {
        return fmt::format("(w={}, h={})", size.width, size.height);
    }

#pragma pack()
}
