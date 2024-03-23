#pragma once

#include <utility>

#include <fmt/format.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"

namespace rl::ds {
#pragma pack(4)
    template <rl::numeric T>
    struct vector2;

    template <rl::numeric T>
    struct dims
    {
    public:
        constexpr dims() = default;
        constexpr ~dims() = default;

        constexpr dims(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

        constexpr dims(const dims<T>& other)
            : width{ other.width }
            , height{ other.height }
        {
        }

        explicit constexpr dims(const vector2<T>& other)
            : width{ other.x }
            , height{ other.y }
        {
        }

        constexpr dims(dims<T>&& other) noexcept
            : width{ std::move(other.width) }
            , height{ std::move(other.height) }
        {
        }

        explicit consteval dims(vector2<T>&& other) noexcept
            : width{ std::move(other.x) }
            , height{ std::move(other.y) }
        {
        }

        constexpr static dims<T> null()
        {
            return dims<T>{
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        constexpr static dims<T> zero()
        {
            return dims<T>{
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        [[nodiscard]]
        constexpr T area() const
        {
            return width * height;
        }

        template <typename I>
            requires rl::floating_point<T>
        explicit constexpr operator ds::dims<I>()
        {
            return ds::dims<I>{
                static_cast<I>(width),
                static_cast<I>(height),
            };
        }

        template <typename F>
            requires rl::integer<T>
        explicit constexpr operator ds::dims<F>()
        {
            return ds::dims<F>{
                static_cast<F>(width),
                static_cast<F>(height),
            };
        }

        constexpr dims<T>& operator=(const dims<T>& other)
        {
            this->height = other.height;
            this->width = other.width;
            return *this;
        }

        constexpr dims<T>& operator=(dims<T>&& other) noexcept
        {
            this->height = std::move(other.height);
            this->width = std::move(other.width);
            return *this;
        }

        constexpr bool operator==(const dims<T>& other) const
        {
            return this->height == other.height && this->width == other.width;
        }

        constexpr bool operator!=(const dims<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr dims<T> operator+(const T val) const
        {
            return dims<T>{
                this->width + val,
                this->height + val,
            };
        }

        constexpr dims<T> operator+(const dims<T>& other) const
        {
            return dims<T>{
                static_cast<T>(this->width + other.width),
                static_cast<T>(this->height + other.height),
            };
        }

        constexpr dims<T> operator+(const vector2<T>& other) const noexcept
        {
            return dims<T>{
                static_cast<T>(this->width + other.x),
                static_cast<T>(this->height + other.y),
            };
        }

        constexpr dims<T> operator+(vector2<T>&& other) const noexcept
        {
            return dims<T>{
                static_cast<T>(this->width + other.x),
                static_cast<T>(this->height + other.y),
            };
        }

        constexpr dims<T>& operator+=(const T& val)
        {
            this->width += val;
            this->height += val;
            return *this;
        }

        constexpr dims<T> operator-(const dims<T>& other) const
        {
            return {
                this->width - other.width,
                this->height - other.height,
            };
        }

        constexpr dims<T> operator-(const vector2<T>& other) const
        {
            return {
                this->width - other.x,
                this->height - other.y,
            };
        }

        constexpr dims<T> operator-(dims<T>&& other) const noexcept
        {
            return {
                this->width - other.width,
                this->height - other.height,
            };
        }

        template <rl::numeric V>
        constexpr dims<T> operator-(const V& val) const
        {
            return dims<T>{
                this->width - static_cast<T>(val),
                this->height - static_cast<T>(val),
            };
        }

        constexpr dims<T>& operator-=(const dims<T>& other)
        {
            this->width -= other.width;
            this->height -= other.height;
            return *this;
        }

        constexpr dims<T>& operator-=(const T& val)
        {
            this->width -= val;
            this->height -= val;
            return *this;
        }

        template <rl::numeric N>
        constexpr ds::dims<N> operator/(const N& val) const
        {
            return ds::dims<N>{
                static_cast<N>(this->width / val),
                static_cast<N>(this->height / val),
            };
        }

        consteval ds::dims<T> operator/(const vector2<T>& vec) const
        {
            return ds::dims<T>{
                static_cast<T>(this->width / vec.x),
                static_cast<T>(this->height / vec.y),
            };
        }

        consteval dims<T> operator/(dims<T>&& other) noexcept
        {
            return dims<T>{
                static_cast<T>(this->width / other.width),
                static_cast<T>(this->height / other.height),
            };
        }

        constexpr dims<T> operator/(const dims<T>& other) const
        {
            return dims<T>{
                static_cast<T>(this->width / other.width),
                static_cast<T>(this->height / other.height),
            };
        }

        constexpr dims<T>& operator/=(const T& val)
        {
            this->width /= val;
            this->height /= val;
            return *this;
        }

        constexpr dims<T> operator*(const T& val) const
        {
            return dims<T>{
                static_cast<T>(this->width * val),
                static_cast<T>(this->height * val),
            };
        }

        constexpr dims<T> operator*(const vector2<T>& vec) const
        {
            return dims<T>{
                static_cast<T>(this->width * vec.x),
                static_cast<T>(this->height * vec.y),
            };
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr dims<T> operator*(vector2<I>&& vec) const noexcept
        {
            return dims<T>{
                this->width * static_cast<T>(vec.x),
                this->height * static_cast<T>(vec.y),
            };
        }

        constexpr dims<T>& operator*=(const T& val)
        {
            this->width *= cast::to<T>(val);
            this->height *= cast::to<T>(val);
            return *this;
        }

    public:
        T width{ static_cast<T>(0) };
        T height{ static_cast<T>(0) };
    };

    template <rl::numeric T>
    constexpr auto format_as(const dims<T>& size)
    {
        return fmt::format("(w={}, h={})", size.width, size.height);
    }

#pragma pack()
}
