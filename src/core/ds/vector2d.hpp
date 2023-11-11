#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#include "core/ds/dimensions.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/concepts.hpp"
#include "core/utils/conversions.hpp"

namespace SDL3
{
#include <SDL3/SDL_rect.h>
}

namespace rl::ds
{

    template <rl::numeric T>
    struct vector2
    {
        explicit constexpr vector2()
            : x{ std::numeric_limits<T>::max() }
            , y{ std::numeric_limits<T>::max() }
        {
        }

        constexpr vector2(const T _x, const T _y)
            : x{ _x }
            , y{ _y }
        {
        }

        template <rl::integer I>
        constexpr vector2(const vector2<I>& other)
            requires rl::floating_point<T>
            : x(cast::to<T>(other.x))
            , y(cast::to<T>(other.y))
        {
        }

        constexpr vector2(const vector2<T>& other)
            : x(other.x)
            , y(other.y)
        {
        }

        constexpr vector2(vector2<T>&& other)
            : x(std::forward<T>(other.x))
            , y(std::forward<T>(other.y))
        {
        }

        constexpr vector2(const SDL3::SDL_Point& pt)
            requires std::same_as<T, i32>
            : x{ pt.x }
            , y{ pt.y }
        {
        }

        constexpr vector2(SDL3::SDL_Point&& pt)
            requires std::same_as<T, i32>
            : x{ pt.x }
            , y{ pt.y }
        {
        }

        constexpr operator SDL3::SDL_Point()
            requires std::same_as<T, i32>
        {
            return *reinterpret_cast<SDL3::SDL_Point*>(this);
        }

        constexpr operator SDL3::SDL_FPoint()
            requires std::same_as<T, f32>
        {
            return *reinterpret_cast<SDL3::SDL_FPoint*>(this);
        }

        constexpr operator SDL3::SDL_Point*()
            requires std::same_as<T, i32>
        {
            return reinterpret_cast<SDL3::SDL_Point*>(this);
        }

        constexpr operator SDL3::SDL_FPoint*()
            requires std::same_as<T, f32>
        {
            return reinterpret_cast<SDL3::SDL_FPoint*>(this);
        }

        constexpr operator const SDL3::SDL_Point() const
            requires std::same_as<T, i32>
        {
            return *reinterpret_cast<const SDL3::SDL_Point*>(this);
        }

        constexpr operator const SDL3::SDL_FPoint() const
            requires std::same_as<T, f32>
        {
            return *reinterpret_cast<const SDL3::SDL_FPoint*>(this);
        }

        constexpr operator const SDL3::SDL_Point*() const
            requires std::same_as<T, i32>
        {
            return reinterpret_cast<const SDL3::SDL_Point*>(this);
        }

        constexpr operator const SDL3::SDL_FPoint*() const
            requires std::same_as<T, f32>
        {
            return reinterpret_cast<const SDL3::SDL_FPoint*>(this);
        }

        inline static constexpr vector2<T> null()
        {
            return vector2<T>{};
        }

        inline static constexpr vector2<T> zero()
        {
            return vector2<T>{
                cast::to<T>(0),
                cast::to<T>(0),
            };
        }

        inline constexpr bool is_zero(bool exact = false) noexcept
        {
            if (exact)
                return this->operator==(vector2<T>::zero());
            else
            {
                return std::abs(x) < std::numeric_limits<T>::epsilon() &&
                       std::abs(y) < std::numeric_limits<T>::epsilon();
            }
        }

        inline constexpr f32 length() const
        {
            return std::sqrtf(this->length_squared());
        }

        inline constexpr f32 length_squared() const
        {
            return x * x + y * y;
        }

        inline constexpr vector2<T> clamped_length(const f32 maxlen) const
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

        inline constexpr f32 distance_squared(const vector2<T>& other) const
        {
            return ((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y));
        }

        inline constexpr f32 distance(const vector2<T>& other) const
        {
            return sqrt(this->distance_squared(other));
        }

        inline constexpr f32 angle_to_vec(const vector2<T>& other) const
        {
            return atan2(this->cross_product(other), this->dot_product(other));
        }

        inline constexpr f32 angle_to_point(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        inline constexpr const vector2<T>& normalize()
        {
            f32 len_sq = this->length_squared();
            if (len_sq != 0.0f)
            {
                f32 len = std::sqrtf(len_sq);
                x /= len;
                y /= len;
            }
        }

        inline constexpr vector2<T> normalized() const
        {
            vector2<T> ret{ x, y };
            return ret.normalize();
        }

        inline constexpr f32 angle() const
        {
            return std::atan2f(y, x);
        }

        inline static constexpr vector2<T> from_angle(const f32 angle)
        {
            return {
                std::cosf(angle),
                std::sinf(angle),
            };
        }

        inline constexpr f32 angle_to(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        inline constexpr f32 dot_product(const vector2& other) const
        {
            return (x * other.x) + (y * other.y);
        }

        inline constexpr f32 cross_product(const vector2& other) const
        {
            return (x * other.y) - (y * other.x);
        }

        inline constexpr vector2<T> rotated(const f32 radians) const
        {
            f32 s{ std::sin(radians) };
            f32 c{ std::cos(radians) };
            return {
                (x * c) - (y * s),
                (x * s) + (y * c),
            };
        }

        inline constexpr vector2<T> clamp(const vector2<T>& min, const vector2<T>& max) const
        {
            return {
                std::clamp(x, min.x, max.x),
                std::clamp(y, min.y, max.y),
            };
        }

        inline constexpr bool operator==(const vector2<T>& other) const
        {
            return x == other.x && y == other.y;
        }

        inline constexpr bool operator==(const vector2<T>& other) const
            requires rl::floating_point<T>
        {
            return std::abs(x - other.x) <= std::numeric_limits<T>::epsilon() &&
                   std::abs(y - other.y) <= std::numeric_limits<T>::epsilon();
        }

        inline constexpr bool operator!=(const vector2<T>& other) const
        {
            return x != other.x || y != other.y;
        }

        inline constexpr vector2<T> lerp(const vector2<T>& to, const f32 weight) const
        {
            vector2<T> ret{ *this };
            ret.x = std::lerp(ret.x, to.x, weight);
            ret.y = std::lerp(ret.y, to.y, weight);
            return ret;
        }

        inline constexpr vector2<T> slerp(const vector2<T>& to, const f32 weight) const
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

        inline constexpr vector2<T> move_towards(const vector2<T>& target, const f32 delta) const
        {
            vector2<T> vec_delta{ target - *this };

            f32 vd_len = vec_delta.length();
            return vd_len <= delta || vd_len < std::numeric_limits<f32>::epsilon()
                       ? target
                       : (*this + vec_delta) / (vd_len * delta);
        }

        inline constexpr vector2<T> slide(const vector2<T>& normal) const
        {
            return { *this - (normal * this->dot(normal)) };
        }

        inline constexpr vector2<T> reflect(const vector2<T>& normal) const
        {
            return { (2.0f * normal * this->dot_product(normal)) - *this };
        }

        inline constexpr vector2<T> bounce(const vector2<T>& normal) const
        {
            return -this->reflect(normal);
        }

        inline constexpr vector2<T> operator+(const vector2<T>& other) const
        {
            return {
                x + other.x,
                y + other.y,
            };
        }

        inline constexpr vector2<T> operator+=(const vector2<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        template <typename V>
        inline constexpr vector2<T> operator-(const vector2<V>& other) const
        {
            return {
                x - cast::to<T>(other.x),
                y - cast::to<T>(other.y),
            };
        }

        inline constexpr vector2<T> operator-(const dimensions<T>& other) const
        {
            return {
                x - other.width,
                y - other.height,
            };
        }

        inline constexpr vector2<T> operator-=(const vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        inline constexpr vector2<T> operator-=(const dimensions<T>& other) const
        {
            x -= other.width;
            y -= other.height;
            return *this;
        }

        inline constexpr vector2<T> operator*(const vector2<T>& other) const
        {
            return {
                x * other.x,
                y * other.y,
            };
        }

        inline constexpr vector2<T> operator*(const f32 val) const
        {
            return {
                x * val,
                y * val,
            };
        }

        inline constexpr vector2<T> operator*=(const f32 val)
        {
            x *= val;
            y *= val;
            return *this;
        }

        inline constexpr vector2<T> operator/(const vector2<T>& other) const
        {
            return {
                x / other.x,
                y / other.y,
            };
        }

        inline constexpr vector2<T> operator/(const f32& val) const
        {
            return { x / val, y / val };
        }

        inline constexpr vector2<T> operator/=(const f32& val)
        {
            x /= val;
            y /= val;
            return *this;
        }

        inline constexpr vector2<T> operator-() const
        {
            return {
                -x,
                -y,
            };
        }

        T x{ 0 };
        T y{ 0 };
    };
}
