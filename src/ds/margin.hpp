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
    struct margin
    {
    public:
        constexpr margin() = default;
        constexpr ~margin() = default;

        constexpr margin(T w, T h)
            : horizontal{ w }
            , vertical{ h }
        {
        }

        constexpr margin(const margin<T>& other)
            : horizontal{ other.horizontal }
            , vertical{ other.vertical }
        {
        }

        constexpr margin(const vector2<T>& other)
            : horizontal{ other.x }
            , vertical{ other.y }
        {
        }

        constexpr margin(margin<T>&& other) noexcept
            : horizontal{ std::move(other.horizontal) }
            , vertical{ std::move(other.vertical) }
        {
        }

        consteval margin(vector2<T>&& other) noexcept
            : horizontal{ std::move(other.x) }
            , vertical{ std::move(other.y) }
        {
        }

        template <typename I>
        explicit constexpr operator ds::margin<I>()
            requires rl::floating_point<T>
        {
            return ds::margin<I>{
                static_cast<I>(horizontal),
                static_cast<I>(vertical),
            };
        }

        template <typename F>
        explicit constexpr operator ds::margin<F>()
            requires rl::integer<T>
        {
            return ds::margin<F>{
                static_cast<F>(horizontal),
                static_cast<F>(vertical),
            };
        }

        constexpr margin<T>& operator=(const margin<T>& other)
        {
            this->vertical = other.vertical;
            this->horizontal = other.horizontal;
            return *this;
        }

        constexpr margin<T>& operator=(margin<T>&& other) noexcept
        {
            this->vertical = std::move(other.vertical);
            this->horizontal = std::move(other.horizontal);
            return *this;
        }

        constexpr bool operator==(const margin<T>& other) const
        {
            return this->vertical == other.vertical && this->horizontal == other.horizontal;
        }

        constexpr bool operator!=(const margin<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr margin<T> operator+(const T val) const
        {
            return margin<T>{
                this->horizontal + val,
                this->vertical + val,
            };
        }

        constexpr margin<T> operator+(const margin<T>& other) const
        {
            return margin<T>{
                static_cast<T>(this->horizontal + other.horizontal),
                static_cast<T>(this->vertical + other.vertical),
            };
        }

        constexpr margin<T> operator+(vector2<T>&& other) noexcept
        {
            return margin<T>{
                static_cast<T>(this->horizontal + other.x),
                static_cast<T>(this->vertical + other.y),
            };
        }

        constexpr margin<T>& operator+=(const T& val)
        {
            this->horizontal += val;
            this->vertical += val;
            return *this;
        }

        constexpr margin<T> operator-(const margin<T>& other) const
        {
            return {
                this->horizontal - other.horizontal,
                this->vertical - other.vertical,
            };
        }

        template <rl::numeric V>
        constexpr margin<T> operator-(const V& val) const
        {
            return margin<T>{
                this->horizontal - static_cast<T>(val),
                this->vertical - static_cast<T>(val),
            };
        }

        constexpr margin<T>& operator-=(const margin<T>& other)
        {
            this->horizontal -= other.horizontal;
            this->vertical -= other.vertical;
            return *this;
        }

        constexpr margin<T>& operator-=(const T& val)
        {
            this->horizontal -= val;
            this->vertical -= val;
            return *this;
        }

        template <rl::numeric N>
        constexpr ds::margin<N> operator/(const N& val) const
        {
            return ds::margin<N>{
                static_cast<N>(this->horizontal / val),
                static_cast<N>(this->vertical / val),
            };
        }

        consteval ds::margin<T> operator/(const vector2<T>& vec) const
        {
            return ds::margin<T>{
                static_cast<T>(this->horizontal / vec.x),
                static_cast<T>(this->vertical / vec.y),
            };
        }

        consteval margin<T> operator/(margin<T>&& other) noexcept
        {
            return margin<T>{
                static_cast<T>(this->horizontal / other.horizontal),
                static_cast<T>(this->vertical / other.vertical),
            };
        }

        constexpr margin<T> operator/(const margin<T>& other) const
        {
            return margin<T>{
                static_cast<T>(this->horizontal / other.horizontal),
                static_cast<T>(this->vertical / other.vertical),
            };
        }

        constexpr margin<T>& operator/=(const T& val)
        {
            this->horizontal /= val;
            this->vertical /= val;
            return *this;
        }

        constexpr margin<T> operator*(const T& val) const
        {
            return margin<T>{
                static_cast<T>(this->horizontal * val),
                static_cast<T>(this->vertical * val),
            };
        }

        constexpr margin<T> operator*(const vector2<T>& vec) const
        {
            return margin<T>{
                static_cast<T>(this->horizontal * vec.x),
                static_cast<T>(this->vertical * vec.y),
            };
        }

        constexpr margin<T>& operator*=(const T& val)
        {
            this->horizontal *= cast::to<T>(val);
            this->vertical *= cast::to<T>(val);
            return *this;
        }

    public:
        T horizontal{ static_cast<T>(0) };
        T vertical{ static_cast<T>(0) };
    };

    template <rl::numeric T>
    constexpr auto format_as(const margin<T>& size)
    {
        return fmt::format("margin:[horiz={} vert={}]", size.horizontal, size.vertical);
    }

#pragma pack()
}
