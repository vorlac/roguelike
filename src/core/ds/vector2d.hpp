#pragma once

#include <algorithm>
#include <cmath>
#include <memory>

#include "core/numerics.hpp"
#include "core/utils/concepts.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::ds
{
    template <typename T>
        requires Numeric<T>
    struct vector2
    {
        T x{ 0 };
        T y{ 0 };

        constexpr vector2(const T _x, const T _y)
            : x{ _x }
            , y{ _y }
        {
        }

    public:
        // template <typename V>
        //     requires std::same_as<T, V>
        // constexpr vector2(const raylib::Vector2& other)
        //{
        //     std::memcpy(this, &other, sizeof(*this));
        // }

        // constexpr operator raylib::Vector2() const
        //{
        //     return *reinterpret_cast<const raylib::Vector2*>(this);
        // }

    public:
        constexpr bool is_zero(bool exact = false) noexcept
        {
            if (exact)
                return this->operator==(vector2<T>::zero());
            else
            {
                return std::abs(x) < std::numeric_limits<T>::epsilon() &&
                       std::abs(y) < std::numeric_limits<T>::epsilon();
            }
        }

        static constexpr inline vector2<T> zero() noexcept
        {
            return {
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        constexpr inline f32 length() const
        {
            return std::sqrtf(this->length_squared());
        }

        constexpr inline f32 length_squared() const
        {
            return (x * x + y * y);
        }

        constexpr inline vector2<T> clamped_length(const f32 maxlen) const
        {
            vector2<T> ret{ *this };

            const f32 len = this->length();
            if (len > 0 && maxlen < len)
            {
                ret /= len;
                ret *= maxlen;
            }

            return ret;
        }

        constexpr inline f32 distance_squared(const vector2<T>& other) const
        {
            return ((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y));
        }

        constexpr inline f32 distance(const vector2<T>& other) const
        {
            return sqrt(this->distance_squared(other));
        }

        constexpr inline f32 angle_to_vec(const vector2<T>& other) const
        {
            return atan2(this->cross_product(other), this->dot_product(other));
        }

        constexpr inline f32 angle_to_point(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        constexpr inline const vector2<T>& normalize()
        {
            f32 len_sq = this->length_squared();
            if (len_sq != static_cast<T>(0))
            {
                f32 len = std::sqrtf(len_sq);
                x /= len;
                y /= len;
            }
        }

        constexpr inline vector2<T> normalized() const
        {
            vector2<T> ret{ x, y };
            return ret.normalize();
        }

        constexpr inline f32 angle() const
        {
            return std::atan2f(y, x);
        }

        static inline constexpr vector2<T> from_angle(const f32 angle)
        {
            return {
                std::cosf(angle),
                std::sinf(angle),
            };
        }

        constexpr inline f32 angle_to(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        constexpr inline f32 dot_product(const vector2& other) const
        {
            return (x * other.x) + (y * other.y);
        }

        constexpr inline f32 cross_product(const vector2& other) const
        {
            return (x * other.y) - (y * other.x);
        }

        constexpr inline vector2<T> rotated(const f32 radians) const
        {
            f32 s{ std::sin(radians) };
            f32 c{ std::cos(radians) };
            return {
                (x * c) - (y * s),
                (x * s) + (y * c),
            };
        }

        constexpr inline vector2<T> clamp(const vector2<T>& min, const vector2<T>& max) const
        {
            return {
                std::clamp(x, min.x, max.x),
                std::clamp(y, min.y, max.y),
            };
        }

        constexpr inline bool operator==(const vector2<T>& other) const
        {
            return x == other.x && y == other.y;
        }

        constexpr inline bool operator!=(const vector2<T>& other) const
        {
            return x != other.x || y != other.y;
        }

        constexpr inline vector2<T> lerp(const vector2<T>& to, const f32 weight) const
        {
            vector2<T> ret{ *this };
            ret.x = std::lerp(ret.x, to.x, weight);
            ret.y = std::lerp(ret.y, to.y, weight);
            return ret;
        }

        constexpr inline vector2<T> slerp(const vector2<T>& to, const f32 weight) const
        {
            f32 start_length_sq{ this->length_squared() };
            f32 end_length_sq{ to.length_squared() };

            if (start_length_sq == 0.0f || end_length_sq == 0.0f) [[unlikely]]
            {
                // Zero length vectors have no angle, so the best
                // we can do is either lerp or throw an error.
                return this->lerp(to, weight);
            }

            f32 start_length{ std::sqrtf(start_length_sq) };
            f32 result_length{ std::lerp(start_length, std::sqrtf(end_length_sq), weight) };
            f32 angle = this->angle_to(to);

            return this->rotated(angle * weight) * (result_length / start_length);
        }

        constexpr inline vector2<T> move_towards(const vector2<T>& target, const f32 delta) const
        {
            vector2<T> vec_delta{ target - *this };

            f32 vd_len = vec_delta.length();
            return vd_len <= delta || vd_len < std::numeric_limits<f32>::epsilon()
                       ? target
                       : (*this + vec_delta) / (vd_len * delta);
        }

        constexpr inline vector2<T> slide(const vector2<T>& normal) const
        {
            return { *this - (normal * this->dot(normal)) };
        }

        constexpr inline vector2<T> reflect(const vector2<T>& normal) const
        {
            return { (2.0f * normal * this->dot_product(normal)) - *this };
        }

        constexpr inline vector2<T> bounce(const vector2<T>& normal) const
        {
            return -this->reflect(normal);
        }

    public:
        constexpr inline vector2<T> operator+(const vector2<T>& other) const
        {
            return {
                x + other.x,
                y + other.y,
            };
        }

        constexpr inline void operator+=(const vector2<T>& other)
        {
            x += other.x;
            y += other.y;
        }

        constexpr inline vector2<T> operator-(const vector2<T>& other) const
        {
            return {
                x - other.x,
                y - other.y,
            };
        }

        constexpr inline void operator-=(const vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
        }

        constexpr inline vector2<T> operator*(const vector2<T>& other) const
        {
            return {
                x * other.x,
                y * other.y,
            };
        }

        constexpr inline vector2<T> operator*(const f32 val) const
        {
            return {
                x * val,
                y * val,
            };
        }

        constexpr inline void operator*=(const f32 val)
        {
            x *= val;
            y *= val;
        }

        constexpr inline vector2<T> operator/(const vector2<T>& other) const
        {
            return {
                x / other.x,
                y / other.y,
            };
        }

        constexpr inline vector2<T> operator/(const f32& val) const
        {
            return { x / val, y / val };
        }

        constexpr inline void operator/=(const f32& val)
        {
            x /= val;
            y /= val;
        }

        constexpr inline vector2<T> operator-() const
        {
            return {
                -x,
                -y,
            };
        }
    };
}
