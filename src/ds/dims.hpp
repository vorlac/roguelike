#pragma once

#include <utility>

#include <fmt/format.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/math.hpp"

namespace rl::ds {
#pragma pack(4)
    template <rl::numeric T>
    struct vector2;
    template <rl::numeric T>
    struct margin;

    template <rl::numeric T>
    struct dims
    {
    public:
        [[nodiscard]]
        consteval static dims<T> null()
        {
            return dims{
                static_cast<T>(-1),
                static_cast<T>(-1),
            };
        }

        [[nodiscard]]
        consteval static dims<T> zero()
        {
            return dims{
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        [[nodiscard]]
        constexpr T area() const
        {
            return width * height;
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr operator dims<I>() const
        {
            return dims<I>{
                static_cast<I>(this->width),
                static_cast<I>(this->height),
            };
        }

        template <rl::floating_point F>
            requires rl::integer<T>
        constexpr operator dims<F>() const
        {
            return dims<F>{
                static_cast<F>(this->width),
                static_cast<F>(this->height),
            };
        }

        [[nodiscard]]
        dims<T> merged(const dims<T>& other) const
        {
            dims ret{ *this };
            if (math::equal(this->width, 0.0f))
                ret.width = other.width;
            if (math::equal(this->height, 0.0f))
                ret.height = other.height;
            return ret;
        }

        //////////////////////////////////////////////////

        constexpr bool operator==(const dims<T>& other) const
        {
            return math::equal(this->height, other.height) &&  //
                   math::equal(this->width, other.width);
        }

        constexpr bool operator!=(const dims<T>& other) const
        {
            return !this->operator==(other);
        }

        //////////////////////////////////////////////////

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator+(V val) const
        {
            return dims{
                static_cast<T>(this->width + val),
                static_cast<T>(this->height + val),
            };
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator-(V val) const
        {
            return dims{
                static_cast<T>(this->width - val),
                static_cast<T>(this->height - val),
            };
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator*(V val) const
        {
            return dims{
                static_cast<T>(this->width * val),
                static_cast<T>(this->height * val),
            };
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator/(V val) const
        {
            return dims{
                static_cast<T>(this->width / val),
                static_cast<T>(this->height / val),
            };
        }

        //////////////////////////////////////////////////

        constexpr dims<T> operator+(dims<T> other) const
        {
            return dims{
                static_cast<T>(this->width + other.width),
                static_cast<T>(this->height + other.height),
            };
        }

        constexpr dims<T> operator-(dims<T> other) const
        {
            return dims{
                this->width - other.width,
                this->height - other.height,
            };
        }

        constexpr dims<T> operator*(dims<T> other) const
        {
            return dims{
                this->width * other.width,
                this->height * other.height,
            };
        }

        constexpr dims<T> operator/(dims<T> other) const
        {
            return dims{
                this->width / other.width,
                this->height / other.height,
            };
        }

        //////////////////////////////////////////////////

        constexpr dims<T> operator+(vector2<T> other) const
        {
            return dims{
                this->width + other.x,
                this->height + other.y,
            };
        }

        constexpr dims<T> operator-(vector2<T> other) const
        {
            return dims{
                this->width - other.x,
                this->height - other.y,
            };
        }

        constexpr dims<T> operator*(vector2<T> other) const
        {
            return dims{
                this->width * other.x,
                this->height * other.y,
            };
        }

        constexpr dims<T> operator/(vector2<T> other) const
        {
            return dims{
                this->width / other.x,
                this->height / other.y,
            };
        }

        //////////////////////////////////////////////////

        constexpr dims<T> operator+(margin<T> other) const
        {
            return dims{
                this->width + other.horizontal(),
                this->height + other.vertical(),
            };
        }

        constexpr dims<T> operator-(margin<T> other) const
        {
            return dims{
                this->width - other.horizontal(),
                this->height - other.vertical(),
            };
        }

        //////////////////////////////////////////////////

        constexpr dims<T>& operator+=(T val)
        {
            this->width += val;
            this->height += val;
            return *this;
        }

        constexpr dims<T>& operator-=(T val)
        {
            this->width -= val;
            this->height -= val;
            return *this;
        }

        constexpr dims<T>& operator*=(T val)
        {
            this->width *= val;
            this->height *= val;
            return *this;
        }

        constexpr dims<T>& operator/=(T val)
        {
            this->width /= val;
            this->height /= val;
            return *this;
        }

        //////////////////////////////////////////////////

        constexpr dims<T>& operator+=(const dims<T>& other)
        {
            this->width += other.width;
            this->height += other.height;
            return *this;
        }

        constexpr dims<T>& operator-=(dims<T> other)
        {
            this->width -= other.width;
            this->height -= other.height;
            return *this;
        }

        constexpr dims<T>& operator*=(dims<T> other)
        {
            this->width *= other.width;
            this->height *= other.height;
            return *this;
        }

        constexpr dims<T>& operator/=(dims<T> other)
        {
            this->width /= other.width;
            this->height /= other.height;
            return *this;
        }

        //////////////////////////////////////////////////

        constexpr dims<T>& operator+=(margin<T> other)
        {
            this->width += other.horizontal();
            this->height += other.vertical();
            return *this;
        }

        constexpr dims<T>& operator-=(margin<T> other)
        {
            this->width -= other.horizontal();
            this->height -= other.vertical();
            return *this;
        }

        //////////////////////////////////////////////////

        constexpr dims<T>& operator+=(const vector2<T>& other)
        {
            this->width += other.x;
            this->height += other.y;
            return *this;
        }

        constexpr dims<T>& operator-=(const vector2<T>& other)
        {
            this->width -= other.x;
            this->height -= other.y;
            return *this;
        }

        constexpr dims<T>& operator*=(const vector2<T>& other)
        {
            this->width *= other.x;
            this->height *= other.y;
            return *this;
        }

        constexpr dims<T>& operator/=(const vector2<T>& other)
        {
            this->width /= other.x;
            this->height /= other.y;
            return *this;
        }

        //////////////////////////////////////////////////

        // template <rl::integer I>
        //     requires rl::floating_point<T>
        // constexpr operator dims<I>() const
        //{
        //     return dims{
        //         static_cast<T>(this->width),
        //         static_cast<T>(this->height),
        //     };
        // }

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
