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
        T width{ static_cast<T>(0) };
        T height{ static_cast<T>(0) };

        constexpr inline dims()
            : width{ static_cast<T>(0) }
            , height{ static_cast<T>(0) }
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
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        constexpr static inline dims<T> zero()
        {
            return {
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        constexpr inline T area() const
            requires rl::floating_point<T>
        {
            const T area = width * height;
            constexpr T epsilon = std::numeric_limits<T>::epsilon();
            if (std::abs(area) <= epsilon)
                return static_cast<T>(0);
            return area;
        }

        constexpr inline T area() const
            requires rl::integer<T>
        {
            return width * height;
        }

        template <typename I>
        explicit constexpr inline operator ds::dims<I>()
            requires rl::floating_point<T>
        {
            return ds::dims<I>{
                static_cast<I>(width),
                static_cast<I>(height),
            };
        }

        template <typename F>
        explicit constexpr inline operator ds::dims<F>()
            requires rl::integer<T>
        {
            return ds::dims<F>{
                static_cast<F>(width),
                static_cast<F>(height),
            };
        }

        constexpr inline dims<T>& operator=(const dims<T>& other)
        {
            this->height = other.height;
            this->width = other.width;
            return *this;
        }

        constexpr inline dims<T>& operator=(dims<T>&& other) noexcept
        {
            this->height = std::move(other.height);
            this->width = std::move(other.width);
            return *this;
        }

        constexpr inline bool operator==(const dims<T>& other) const
        {
            return 0 == rl::memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        constexpr inline bool operator!=(const dims<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr inline dims<T> operator+(const T val) const
        {
            return {
                static_cast<T>(this->width + val),
                static_cast<T>(this->height + val),
            };
        }

        constexpr inline dims<T> operator+(const dims<T>& other) const
        {
            return {
                static_cast<T>(this->width + other.width),
                static_cast<T>(this->height + other.height),
            };
        }

        constexpr inline dims<T>& operator+=(const T& val)
        {
            this->width += val;
            this->height += val;
            return *this;
        }

        constexpr inline dims<T> operator-(const dims<T>& other) const
        {
            return {
                this->width - other.width,
                this->height - other.height,
            };
        }

        template <rl::numeric V>
        constexpr inline dims<T> operator-(const V& val) const
        {
            return {
                this->width - cast::to<T>(val),
                this->height - cast::to<T>(val),
            };
        }

        constexpr inline dims<T>& operator-=(const dims<T>& other)
        {
            this->width -= other.width;
            this->height -= other.height;
            return *this;
        }

        constexpr inline dims<T>& operator-=(const T& val)
        {
            this->width -= val;
            this->height -= val;
            return *this;
        }

        template <rl::numeric N>
        constexpr inline ds::dims<N> operator/(const N& val) const
        {
            return ds::dims{
                this->width / val,
                this->height / val,
            };
        }

        constexpr inline dims<T>& operator/=(const T& val)
        {
            this->width /= val;
            this->height /= val;
            return *this;
        }

        constexpr inline dims<T> operator*(const T& val) const
        {
            return dims<T>{
                cast::to<T>(this->width * val),
                cast::to<T>(this->height * val),
            };
        }

        constexpr inline dims<T>& operator*=(const T& val)
        {
            this->width *= cast::to<T>(val);
            this->height *= cast::to<T>(val);
            return *this;
        }
    };

    template <rl::numeric T>
    constexpr auto format_as(const dims<T>& size)
    {
        return fmt::format("(w={}, h={})", size.width, size.height);
    }

#pragma pack()
}
