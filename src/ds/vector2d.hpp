#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include "ds/dims.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct vector2
    {
        constexpr ~vector2() = default;

        constexpr vector2() noexcept
            : x{ std::numeric_limits<T>::max() }
            , y{ std::numeric_limits<T>::max() }
            , z{ static_cast<T>(0) }
        {
        }

        constexpr vector2(const T tx, const T ty, const T tz = 0) noexcept
            : x{ static_cast<T>(tx) }
            , y{ static_cast<T>(ty) }
            , z{ static_cast<T>(tz) }
        {
        }

        template <rl::integer I>
        constexpr vector2(const vector2<I>& other) noexcept
            requires rl::floating_point<T>
            : x{ static_cast<T>(other.x) }
            , y{ static_cast<T>(other.y) }
            , z{ static_cast<T>(0) }
        {
        }

        constexpr vector2(const vector2<T>& other) noexcept
            : x{ static_cast<T>(other.x) }
            , y{ static_cast<T>(other.y) }
            , z{ static_cast<T>(0) }
        {
        }

        constexpr vector2(const vector2<T>&& other) noexcept
            : x{ static_cast<T>(other.x) }
            , y{ static_cast<T>(other.y) }
            , z{ static_cast<T>(0) }
        {
        }

        constexpr T* ptr()
        {
            return &x;
        }

        constexpr static vector2<T> null()
        {
            return vector2<T>{};
        }

        constexpr static vector2<T> zero()
        {
            return vector2<T>{ 0, 0 };
        }

        [[nodiscard]] constexpr bool is_zero() const
            requires rl::integer<T>
        {
            return this->operator==(vector2<T>::zero());
        }

        [[nodiscard]] constexpr bool is_zero() const
            requires rl::floating_point<T>
        {
            constexpr T epsilon = std::numeric_limits<T>::epsilon();
            return std::abs(x - y) < epsilon;
        }

        [[nodiscard]] constexpr f32 length() const
        {
            return std::sqrt(this->length_squared());
        }

        [[nodiscard]] constexpr f32 length_squared() const
        {
            return x * x + y * y;
        }

        constexpr vector2<T> clamped_length(const f32 maxlen) const
        {
            vector2<T> ret{ *this };

            const f32 len = this->length();
            if (len > 0.0f && maxlen < len)
            {
                ret /= len;
                ret *= maxlen;
            }

            return ret;
        }

        constexpr f32 distance_squared(const vector2<T>& other) const
        {
            return ((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y));
        }

        constexpr f32 distance(const vector2<T>& other) const
        {
            return sqrt(this->distance_squared(other));
        }

        constexpr f32 angle_to_vec(const vector2<T>& other) const
        {
            return atan2(this->cross_product(other), this->dot_product(other));
        }

        constexpr f32 angle_to_point(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        constexpr const vector2& normalize()
        {
            const f32 len_sq{ this->length_squared() };
            if (len_sq != 0.0f)
            {
                f32 len = std::sqrt(len_sq);
                x /= len;
                y /= len;
            }
            return *this;
        }

        constexpr vector2 normalized() const
        {
            vector2<T> ret{ x, y };
            return ret.normalize();
        }

        [[nodiscard]] constexpr f32 angle() const
        {
            return std::atan2f(y, x);
        }

        constexpr static vector2 from_angle(const f32 angle)
        {
            return {
                std::cosf(angle),
                std::sinf(angle),
            };
        }

        constexpr f32 angle_to(const vector2& pt) const
        {
            return (pt - *this).angle();
        }

        constexpr f32 dot_product(const vector2& other) const
        {
            return (x * other.x) + (y * other.y);
        }

        constexpr f32 cross_product(const vector2& other) const
        {
            return (x * other.y) - (y * other.x);
        }

        constexpr vector2 rotated(const f32 radians) const
        {
            f32 s{ std::sin(radians) };
            f32 c{ std::cos(radians) };

            return {
                (x * c) - (y * s),
                (x * s) + (y * c),
            };
        }

        constexpr vector2 clamp(const vector2& min, const vector2& max) const
        {
            return {
                std::clamp(x, min.x, max.x),
                std::clamp(y, min.y, max.y),
            };
        }

        constexpr bool operator==(const vector2<T>& other) const
        {
            return x == other.x && y == other.y;
        }

        constexpr bool operator==(const vector2<T>& other) const
            requires rl::floating_point<T>
        {
            return std::abs(x - other.x) <= std::numeric_limits<T>::epsilon() &&
                   std::abs(y - other.y) <= std::numeric_limits<T>::epsilon();
        }

        constexpr bool operator!=(const vector2<T>& other) const
        {
            return x != other.x || y != other.y;
        }

        constexpr vector2<T> lerp(const vector2<T>& to, const f32 weight) const
        {
            vector2<T> ret{ *this };
            ret.x = std::lerp(ret.x, to.x, weight);
            ret.y = std::lerp(ret.y, to.y, weight);
            return ret;
        }

        constexpr vector2<T> slerp(const vector2<T>& to, const f32 weight) const
        {
            f32 start_len_sq{ this->length_squared() };
            f32 end_len_sq{ to.length_squared() };

            if (start_len_sq == T(0) || end_len_sq == T(0)) [[unlikely]]
            {
                // zero length vectors have no angle, so the best
                // we can do is either lerp or throw an error.
                return this->lerp(to, weight);
            }

            const f32 start_length{ std::sqrt(start_len_sq) };
            const f32 result_length{ std::lerp(start_length, std::sqrt(end_len_sq), weight) };
            const f32 angle = this->angle_to(to);

            return this->rotated(angle * weight) * (result_length / start_length);
        }

        constexpr vector2<T> move_towards(const vector2<T>& target, const f32 delta) const
        {
            vector2<T> vec_delta{ target - *this };

            const f32 vd_len = vec_delta.length();
            return vd_len <= delta || vd_len < std::numeric_limits<f32>::epsilon()
                     ? target
                     : (*this + vec_delta) / (vd_len * delta);
        }

        constexpr vector2<T> slide(const vector2<T>& normal) const
        {
            return { *this - (normal * this->dot_product(normal)) };
        }

        constexpr vector2<T> reflect(const vector2<T>& normal) const
        {
            return { (cast::to<T>(2.0) * normal * this->dot_product(normal)) - *this };
        }

        constexpr vector2<T> bounce(const vector2<T>& normal) const
        {
            return -this->reflect(normal);
        }

        constexpr vector2<T>& operator=(const vector2<T>& other)
        {
            this->x = other.x;
            this->y = other.y;
            return *this;
        }

        constexpr vector2<T>& operator=(vector2<T>&& other) noexcept
        {
            this->x = other.x;
            this->y = other.y;
            return *this;
        }

        constexpr vector2<T> operator+(const T& other)
        {
            return vector2<T>{
                this->x + other,
                this->y + other,
            };
        }

        constexpr vector2<T> operator+(vector2<T>&& other) noexcept
        {
            return vector2<T>{
                this->x + std::move(other.x),
                this->y + std::move(other.y),
            };
        }

        constexpr vector2<T> operator+(dims<T>&& other) const noexcept
        {
            return vector2{
                static_cast<T>(this->x + std::move(other.width)),
                static_cast<T>(this->y + std::move(other.height)),
            };
        }

        constexpr vector2<T> operator+=(const vector2<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        template <typename V>
        constexpr vector2<T> operator-(const vector2<V>& other) const
        {
            return vector2<T>{
                x - static_cast<T>(other.x),
                y - static_cast<T>(other.y),
            };
        }

        constexpr vector2<T> operator-(const T& other) const
        {
            return vector2<T>{
                x - static_cast<T>(other),
                y - static_cast<T>(other),
            };
        }

        constexpr vector2<T> operator-(const dims<T>& other) const
        {
            return vector2<T>{
                x - other.width,
                y - other.height,
            };
        }

        constexpr vector2<T>& operator-=(const T& other)
        {
            x -= other;
            y -= other;
            return *this;
        }

        constexpr vector2<T>& operator-=(const vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr vector2<T> operator-=(const dims<T>& other) const
        {
            x -= other.width;
            y -= other.height;
            return *this;
        }

        constexpr vector2<T> operator*(const vector2<T>& other) const
        {
            return vector2<T>{
                x * other.x,
                y * other.y,
            };
        }

        constexpr vector2<T> operator*(const T val) const
        {
            return vector2<T>{
                static_cast<T>(x * val),
                static_cast<T>(y * val),
            };
        }

        constexpr vector2<T> operator*=(const f32 val)
        {
            x *= val;
            y *= val;
            return *this;
        }

        constexpr vector2<T> operator/(const vector2<T>& other) const
        {
            return vector2<T>{
                x / other.x,
                y / other.y,
            };
        }

        constexpr vector2<T> operator/(dims<T>&& other) noexcept
        {
            return vector2<T>{
                static_cast<T>(this->x / std::move(other.width)),
                static_cast<T>(this->y / std::move(other.height)),
            };
        }

        constexpr vector2<T> operator/(const f32& val) const
        {
            return vector2<T>{
                x / val,
                y / val,
            };
        }

        constexpr vector2<T> operator/=(const f32& val)
        {
            x /= val;
            y /= val;
            return *this;
        }

        constexpr vector2<T> operator-() const
        {
            return vector2<T>{
                -x,
                -y,
            };
        }

        T x{};
        T y{};
        T z{};
    };

    template <rl::numeric T>
    constexpr auto format_as(const ds::vector2<T>& vec)
    {
        return fmt::format("(x={}, y={})", vec.x, vec.y);
    }
}

#pragma pack()
