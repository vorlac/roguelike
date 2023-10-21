#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>

#include "core/utils/concepts.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::ds
{
    template <typename T = float>
        requires Numeric<T>
    struct vector2
    {
        T x{ 0 };
        T y{ 0 };

        ~vector2()
        {
        }

        vector2(const T _x, const T _y)
            : x{ _x }
            , y{ _y }
        {
        }

    public:
        template <typename V>
            requires std::same_as<T, V>
        vector2(const raylib::Vector2& other)
        {
            std::memcpy(this, &other, sizeof(*this));
        }

        operator raylib::Vector2() const
        {
            return *reinterpret_cast<const raylib::Vector2*>(this);
        }

    public:
        inline float length() const
        {
            return std::sqrtf(this->length_squared());
        }

        inline float length_squared() const
        {
            return (x * x + y * y);
        }

        inline vector2<T> clamped_length(const float maxlen) const
        {
            vector2<T> ret{ *this };

            const float len = this->length();
            if (len > 0 && maxlen < len)
            {
                ret /= len;
                ret *= maxlen;
            }

            return ret;
        }

        inline float distance_squared(const vector2<T>& other) const
        {
            return ((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y));
        }

        inline float distance(const vector2<T>& other) const
        {
            return sqrt(this->distance_squared(other));
        }

        inline float angle_to_vec(const vector2<T>& other) const
        {
            return atan2(this->cross_product(other), this->dot_product(other));
        }

        inline float angle_to_point(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        inline const vector2<T>& normalize()
        {
            float length_sq = this->length_squared();
            if (length_sq != 0)
            {
                float len = std::sqrtf(length_sq);
                x /= len;
                y /= len;
            }
        }

        inline vector2<T> normalized() const
        {
            vector2<T> ret{ x, y };
            return ret.normalize();
        }

        inline float angle() const
        {
            return std::atan2f(y, x);
        }

        static inline vector2<T> from_angle(const float angle)
        {
            return {
                std::cosf(angle),
                std::sinf(angle),
            };
        }

        inline float angle_to(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        inline float dot_product(const vector2& other) const
        {
            return (x * other.x) + (y * other.y);
        }

        inline float cross_product(const vector2& other) const
        {
            return (x * other.y) - (y * other.x);
        }

        inline vector2<T> rotated(const float radians) const
        {
            float s{ std::sin(radians) };
            float c{ std::cos(radians) };
            return {
                (x * c) - (y * s),
                (x * s) + (y * c),
            };
        }

        inline vector2<T> clamp(const vector2<T>& min, const vector2<T>& max) const
        {
            return {
                std::clamp(x, min.x, max.x),
                std::clamp(y, min.y, max.y),
            };
        }

        inline bool operator==(const vector2<T>& other) const
        {
            return x == other.x && y == other.y;
        }

        inline bool operator!=(const vector2<T>& other) const
        {
            return x != other.x || y != other.y;
        }

        inline vector2<T> lerp(const vector2<T>& to, const float weight) const
        {
            vector2<T> ret{ *this };
            ret.x = std::lerp(ret.x, to.x, weight);
            ret.y = std::lerp(ret.y, to.y, weight);
            return ret;
        }

        inline vector2<T> slerp(const vector2<T>& to, const float weight) const
        {
            float start_length_sq = this->length_squared();
            float end_length_sq = to.length_squared();
            if (start_length_sq == 0.0f || end_length_sq == 0.0f) [[unlikely]]
            {
                // Zero length vectors have no angle, so the best
                // we can do is either lerp or throw an error.
                return this->lerp(to, weight);
            }
            float start_length = std::sqrtf(start_length_sq);
            float result_length = std::lerp(start_length, std::sqrtf(end_length_sq), weight);
            float angle = this->angle_to(to);
            return this->rotated(angle * weight) * (result_length / start_length);
        }

        inline vector2<T> move_towards(const vector2<T>& target, const float delta) const
        {
            vector2<T> vec_delta{ target - *this };

            float vd_len = vec_delta.length();
            return vd_len <= delta || vd_len < std::numeric_limits<float>::epsilon()
                     ? target
                     : (*this + vec_delta) / (vd_len * delta);
        }

        inline vector2<T> slide(const vector2<T>& normal) const
        {
            return { *this - (normal * this->dot(normal)) };
        }

        inline vector2<T> reflect(const vector2<T>& normal) const
        {
            return { (2.0f * normal * this->dot_product(normal)) - *this };
        }

        inline vector2<T> bounce(const vector2<T>& normal) const
        {
            return -this->reflect(normal);
        }

    public:
        inline vector2<T> operator+(const vector2<T>& other) const
        {
            return {
                x + other.x,
                y + other.y,
            };
        }

        inline void operator+=(const vector2<T>& other)
        {
            x += other.x;
            y += other.y;
        }

        inline vector2<T> operator-(const vector2<T>& other) const
        {
            return {
                x - other.x,
                y - other.y,
            };
        }

        inline void operator-=(const vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
        }

        inline vector2<T> operator*(const vector2<T>& other) const
        {
            return {
                x * other.x,
                y * other.y,
            };
        }

        inline vector2<T> operator*(const float val) const
        {
            return {
                x * val,
                y * val,
            };
        }

        inline void operator*=(const float val)
        {
            x *= val;
            y *= val;
        }

        inline vector2<T> operator/(const vector2<T>& other) const
        {
            return {
                x / other.x,
                y / other.y,
            };
        }

        inline vector2<T> operator/(const float& val) const
        {
            return { x / val, y / val };
        }

        inline void operator/=(const float& val)
        {
            x /= val;
            y /= val;
        }

        inline vector2<T> operator-() const
        {
            return {
                -x,
                -y,
            };
        }
    };
}
