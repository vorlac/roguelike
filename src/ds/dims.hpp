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
    struct vector2;

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

        constexpr inline dims(const vector2<T>& other)
            : width{ other.x }
            , height{ other.y }
        {
        }

        constexpr inline dims(dims<T>&& other) noexcept
            : width{ std::forward<T>(other.width) }
            , height{ std::forward<T>(other.height) }
        {
        }

        consteval inline dims(vector2<T>&& other) noexcept
            : width{ std::forward<T>(other.x) }
            , height{ std::forward<T>(other.y) }
        {
        }

        constexpr static inline dims<T> null()
        {
            return dims<T>{
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        constexpr static inline dims<T> zero()
        {
            return dims<T>{
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        constexpr inline T area() const
            requires rl::floating_point<T>
        {
            const T area{ width * height };
            constexpr T epsilon{ std::numeric_limits<T>::epsilon() };
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
            return this->height == other.height && this->width == other.width;
        }

        constexpr inline bool operator!=(const dims<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr inline dims<T> operator+(const T val) const
        {
            return dims<T>{
                this->width + val,
                this->height + val,
            };
        }

        constexpr inline dims<T> operator+(const dims<T>& other) const
        {
            return dims<T>{
                static_cast<T>(this->width + other.width),
                static_cast<T>(this->height + other.height),
            };
        }

        constexpr inline dims<T> operator+(vector2<T>&& other) noexcept
        {
            return dims<T>{
                static_cast<T>(this->width + other.x),
                static_cast<T>(this->height + other.y),
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
            return dims<T>{
                this->width - static_cast<T>(val),
                this->height - static_cast<T>(val),
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
            return ds::dims<N>{
                static_cast<N>(this->width / val),
                static_cast<N>(this->height / val),
            };
        }

        consteval inline ds::dims<T> operator/(const vector2<T>& vec) const
        {
            return ds::dims<T>{
                static_cast<T>(this->width / vec.x),
                static_cast<T>(this->height / vec.y),
            };
        }

        consteval inline dims<T> operator/(dims<T>&& other) noexcept
        {
            return dims<T>{
                static_cast<T>(this->width / other.width),
                static_cast<T>(this->height / other.height),
            };
        }

        constexpr inline dims<T> operator/(const dims<T>& other) const
        {
            return dims<T>{
                static_cast<T>(this->width / other.width),
                static_cast<T>(this->height / other.height),
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
                static_cast<T>(this->width * val),
                static_cast<T>(this->height * val),
            };
        }

        constexpr inline dims<T> operator*(const vector2<T>& vec) const
        {
            return dims<T>{
                static_cast<T>(this->width * vec.x),
                static_cast<T>(this->height * vec.y),
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
