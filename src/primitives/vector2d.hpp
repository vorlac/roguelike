#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#include <fmt/format.h>

#include "core/numeric_types.hpp"
#include "ecs/components/transform_components.hpp"
#include "primitives/dims.hpp"
#include "primitives/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_rect.h>
SDL_C_LIB_END

namespace rl::ds {
    template <rl::numeric T>
    struct vector2
    {
        explicit inline constexpr vector2()
            : m_coords{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), 0 }
        {
        }

        constexpr inline vector2(const T _x, const T _y)
            : m_coords{ _x, _y, 0 }
        {
        }

        template <rl::integer I>
        constexpr inline vector2(const vector2<I>& other)
            requires rl::floating_point<T>
            : m_coords{ cast::to<T>(other.x), cast::to<T>(other.y), 0 }
        {
        }

        constexpr inline vector2(const vector2<T>& other)
            : m_coords{ other.x, other.y, 0 }
        {
        }

        constexpr inline vector2(vector2<T>&& other) noexcept
            : m_coords{ std::forward<std::array<T, 3>>(other.m_coords) }
        {
        }

        constexpr inline vector2(const rl::component::position& pos)
            requires std::same_as<T, f32>
            : m_coords{ pos.x, pos.y, 0 }
        {
        }

        constexpr inline vector2(const SDL3::SDL_Point& pt)
            requires std::same_as<T, i32>
            : m_coords{ pt.x, pt.y, 0 }
        {
        }

        constexpr inline vector2(SDL3::SDL_Point&& pt) noexcept
            requires std::same_as<T, i32>
            : m_coords{ pt.x, pt.y, 0 }
        {
        }

        constexpr inline operator std::array<T, 3>()
        {
            return m_coords;
        }

        constexpr inline operator std::array<T, 3>*()
        {
            return m_coords.data();
        }

        constexpr inline operator SDL3::SDL_Point()
            requires std::same_as<T, i32>
        {
            return *reinterpret_cast<SDL3::SDL_Point*>(m_coords.data());
        }

        constexpr inline operator SDL3::SDL_FPoint()
            requires std::same_as<T, f32>
        {
            return *reinterpret_cast<SDL3::SDL_FPoint*>(m_coords.data());
        }

        constexpr inline operator SDL3::SDL_Point*()
            requires std::same_as<T, i32>
        {
            return reinterpret_cast<SDL3::SDL_Point*>(m_coords.data());
        }

        constexpr inline operator SDL3::SDL_FPoint*()
            requires std::same_as<T, f32>
        {
            return reinterpret_cast<SDL3::SDL_FPoint*>(m_coords.data());
        }

        constexpr inline operator const SDL3::SDL_Point() const
            requires std::same_as<T, i32>
        {
            return *reinterpret_cast<const SDL3::SDL_Point*>(m_coords.data());
        }

        constexpr inline operator const SDL3::SDL_FPoint() const
            requires std::same_as<T, f32>
        {
            return *reinterpret_cast<const SDL3::SDL_FPoint*>(m_coords.data());
        }

        constexpr inline operator const SDL3::SDL_Point*() const
            requires std::same_as<T, i32>
        {
            return reinterpret_cast<const SDL3::SDL_Point*>(m_coords.data());
        }

        constexpr inline operator const SDL3::SDL_FPoint*() const
            requires std::same_as<T, f32>
        {
            return reinterpret_cast<const SDL3::SDL_FPoint*>(m_coords.data());
        }

        constexpr inline operator rl::component::position()
            requires std::same_as<T, f32>
        {
            return *static_cast<rl::component::position*>(m_coords.data());
        }

        constexpr inline operator const rl::component::position() const
            requires std::same_as<T, f32>
        {
            return *static_cast<const rl::component::position*>(m_coords.data());
        }

        constexpr static inline vector2<T> null()
        {
            return vector2<T>{};
        }

        constexpr static inline vector2<T> zero()
        {
            return vector2<T>{ 0, 0 };
        }

        constexpr inline bool is_zero() const
            requires rl::integer<T>
        {
            return this->operator==(vector2<T>::zero());
        }

        constexpr inline bool is_zero() const
            requires rl::floating_point<T>
        {
            constexpr T epsilon = std::numeric_limits<T>::epsilon();
            return std::abs(x - y) < epsilon;
        }

        constexpr inline f32 length() const
        {
            return std::sqrtf(this->length_squared());
        }

        constexpr inline f32 length_squared() const
        {
            return x * x + y * y;
        }

        constexpr inline vector2<T> clamped_length(const f32 maxlen) const
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

        constexpr const inline vector2<T>& normalize()
        {
            f32 len_sq = this->length_squared();
            if (len_sq != 0.0f)
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

        constexpr static inline vector2<T> from_angle(const f32 angle)
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

        constexpr inline bool operator==(const vector2<T>& other) const
            requires rl::floating_point<T>
        {
            return std::abs(x - other.x) <= std::numeric_limits<T>::epsilon() &&
                   std::abs(y - other.y) <= std::numeric_limits<T>::epsilon();
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
            f32 start_len_sq{ this->length_squared() };
            f32 end_len_sq{ to.length_squared() };

            if (start_len_sq == cast::to<T>(0.0) || end_len_sq == cast::to<T>(0.0)) [[unlikely]]
            {
                // Zero length vectors have no angle, so the best
                // we can do is either lerp or throw an error.
                return this->lerp(to, weight);
            }

            f32 start_length{ std::sqrtf(start_len_sq) };
            f32 result_length{ std::lerp(start_length, std::sqrtf(end_len_sq), weight) };
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
            return { (cast::to<T>(2.0) * normal * this->dot_product(normal)) - *this };
        }

        constexpr inline vector2<T> bounce(const vector2<T>& normal) const
        {
            return -this->reflect(normal);
        }

        constexpr inline vector2<T>& operator=(const vector2<T>& other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }

        constexpr inline vector2<T> operator+(const vector2<T>& other) const
        {
            return {
                x + other.x,
                y + other.y,
            };
        }

        constexpr inline vector2<T> operator+=(const vector2<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        template <typename V>
        constexpr inline vector2<T> operator-(const vector2<V>& other) const
        {
            return {
                x - cast::to<T>(other.x),
                y - cast::to<T>(other.y),
            };
        }

        constexpr inline vector2<T> operator-(const dims<T>& other) const
        {
            return {
                x - other.width,
                y - other.height,
            };
        }

        constexpr inline vector2<T> operator-=(const vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr inline vector2<T> operator-=(const dims<T>& other) const
        {
            x -= other.width;
            y -= other.height;
            return *this;
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

        constexpr inline vector2<T> operator*=(const f32 val)
        {
            x *= val;
            y *= val;
            return *this;
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

        constexpr inline vector2<T> operator/=(const f32& val)
        {
            x /= val;
            y /= val;
            return *this;
        }

        constexpr inline vector2<T> operator-() const
        {
            return {
                -x,
                -y,
            };
        }

        std::array<T, 3> m_coords{ 0, 0, 0 };
        T& x{ m_coords[0] };
        T& y{ m_coords[1] };
    };

    template <rl::numeric T>
    constexpr inline auto format_as(const ds::vector2<T>& vec)
    {
        return fmt::format("(x={}, y={})", vec.x, vec.y);
    }
}
