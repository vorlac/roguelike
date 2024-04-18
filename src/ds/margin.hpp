#pragma once

#include <fmt/format.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"

namespace rl::ds {
#pragma pack(4)
    template <rl::numeric T>
    struct vector2;

    template <rl::numeric T>
    struct margin
    {
    public:
        [[nodiscard]]
        consteval static margin<T> zero()
        {
            return margin{
                static_cast<T>(0),
                static_cast<T>(0),
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        [[nodiscard]]
        consteval static margin<T> null()
        {
            return margin{
                static_cast<T>(-1),
                static_cast<T>(-1),
                static_cast<T>(-1),
                static_cast<T>(-1),
            };
        }

        [[nodiscard]]
        consteval static margin<T> init(T val)
        {
            return margin{
                static_cast<T>(val),
                static_cast<T>(val),
                static_cast<T>(val),
                static_cast<T>(val),
            };
        }

        [[nodiscard]]
        consteval static margin<T> init(T top, T bot, T lft, T rgt)
        {
            return margin{
                static_cast<T>(top),
                static_cast<T>(bot),
                static_cast<T>(lft),
                static_cast<T>(rgt),
            };
        }

        [[nodiscard]]
        constexpr vector2<T> offset() const
        {
            return vector2{ left, top };
        }

        [[nodiscard]]
        constexpr T vertical() const
        {
            return top + bottom;
        }

        [[nodiscard]]
        constexpr T horizontal() const
        {
            return left + right;
        }

        constexpr bool operator==(const margin<T> other) const
        {
            return math::equal(this->top, other.top)
                && math::equal(this->bottom, other.bottom)
                && math::equal(this->left, other.left)
                && math::equal(this->right, other.right);
        }

        constexpr bool operator!=(const margin<T> other) const
        {
            return !this->operator==(other);
        }

        constexpr margin<T> operator-() const
        {
            // TODO: confirm this is better
            //       than *= -1 for float
            margin ret{ *this };
            ret.top = -ret.top;
            ret.bottom = -ret.bottom;
            ret.left = -ret.left;
            ret.right = -ret.right;
            return ret;
        }

        constexpr const margin<T>& operator+=(T val)
        {
            this->top += val;
            this->bottom += val;
            this->left += val;
            this->right += val;
            return *this;
        }

        constexpr const margin<T>& operator-=(T val)
        {
            this->top -= val;
            this->bottom -= val;
            this->left -= val;
            this->right -= val;
            return *this;
        }

        constexpr const margin<T>& operator*=(T val)
        {
            this->top *= val;
            this->bottom *= val;
            this->left *= val;
            this->right *= val;
            return *this;
        }

        constexpr const margin<T>& operator/=(T val)
        {
            this->top /= val;
            this->bottom /= val;
            this->left /= val;
            this->right /= val;
            return *this;
        }

        constexpr const margin<T>& operator+=(const margin<T>& other)
        {
            this->top += other.top;
            this->bottom += other.bottom;
            this->left += other.left;
            this->right += other.right;
            return *this;
        }

        constexpr const margin<T>& operator-=(const margin<T>& other)
        {
            this->top -= other.top;
            this->bottom -= other.bottom;
            this->left -= other.left;
            this->right -= other.right;
            return *this;
        }

        constexpr const margin<T>& operator*=(const margin<T>& other)
        {
            this->top *= other.top;
            this->bottom *= other.bottom;
            this->left *= other.left;
            this->right *= other.right;
            return *this;
        }

        constexpr const margin<T>& operator/=(const margin<T>& other)
        {
            this->top /= other.top;
            this->bottom /= other.bottom;
            this->left /= other.left;
            this->right /= other.right;
            return *this;
        }

        constexpr margin<T> operator+(T val) const
        {
            margin ret{ *this };
            ret += val;
            return ret;
        }

        constexpr margin<T> operator-(T val) const
        {
            margin ret{ *this };
            ret -= val;
            return ret;
        }

        constexpr margin<T> operator*(T val) const
        {
            margin ret{ *this };
            ret *= val;
            return ret;
        }

        constexpr margin<T> operator/(T val) const
        {
            margin ret{ *this };
            ret /= val;
            return ret;
        }

        constexpr margin<T> operator+(const margin<T>& other) const
        {
            margin ret{ *this };
            ret += other;
            return ret;
        }

        constexpr margin<T> operator-(const margin<T>& other) const
        {
            margin ret{ *this };
            ret -= other;
            return ret;
        }

        constexpr margin<T> operator*(const margin<T>& other) const
        {
            margin ret{ *this };
            ret *= other;
            return ret;
        }

        constexpr margin<T> operator/(const margin<T>& other) const
        {
            margin ret{ *this };
            ret /= other;
            return ret;
        }

    public:
        T top{ 0 };
        T bottom{ 0 };
        T left{ 0 };
        T right{ 0 };
    };

#pragma pack()
}

namespace rl::ds {
    template <rl::numeric T>
    constexpr auto format_as(const margin<T> mar)
    {
        return fmt::format("margin=[t:{} b:{} l:{} r:{}]",
                           mar.top, mar.bottom, mar.left, mar.right);
    }
}
