#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include "ds/dims.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    using point = vector2<T>;

    template <rl::numeric T>
    struct vector2
    {
        [[nodiscard]]
        consteval static vector2<T> zero()
        {
            return vector2{
                static_cast<T>(0),
                static_cast<T>(0),
            };
        }

        [[nodiscard]]
        consteval static vector2<T> null()
        {
            return vector2{
                std::numeric_limits<T>::max(),
                std::numeric_limits<T>::max(),
            };
        }

        [[nodiscard]]
        constexpr static vector2<T> from_angle(const f32 angle)
        {
            return vector2{
                std::cosf(angle),
                std::sinf(angle),
            };
        }

        [[nodiscard]]
        constexpr f32 length() const
        {
            return std::sqrt(this->length_squared());
        }

        [[nodiscard]]
        constexpr f32 length_squared() const
        {
            return (x * x) + (y * y);
        }

        [[nodiscard]]
        constexpr vector2<T> clamped_length(const f32 maxlen) const
        {
            vector2 ret{ *this };

            const f32 len{ this->length() };
            if (len > 0.0f && maxlen < len) {
                ret /= len;
                ret *= maxlen;
            }

            return ret;
        }

        [[nodiscard]]
        constexpr f32 distance_squared(const vector2<T>& other) const
        {
            return ((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y));
        }

        [[nodiscard]]
        constexpr f32 distance(const vector2<T>& other) const
        {
            return sqrt(this->distance_squared(other));
        }

        [[nodiscard]]
        constexpr f32 angle_to_vec(const vector2<T>& other) const
        {
            return atan2(this->cross_product(other), this->dot_product(other));
        }

        [[nodiscard]]
        constexpr f32 angle_to_point(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        [[nodiscard]]
        constexpr CompassDirection dir()
        {
            CompassDirection ret{ CompassDirection::None };

            if (this->x > 0)
                ret |= CompassDirection::East;
            else if (this->x < 0)
                ret |= CompassDirection::West;

            if (this->y > 0)
                ret |= CompassDirection::South;
            else if (this->y < 0)
                ret |= CompassDirection::North;

            return ret;
        }

        [[nodiscard]]
        constexpr const vector2<T>& normalize()
        {
            const f32 len_sq{ this->length_squared() };
            if (len_sq != 0.0f) {
                f32 len = std::sqrt(len_sq);
                x /= len;
                y /= len;
            }
            return *this;
        }

        [[nodiscard]]
        constexpr vector2<T> normalized() const
        {
            vector2 ret{ this->x, this->y };
            return ret.normalize();
        }

        [[nodiscard]]
        constexpr f32 angle() const
        {
            return std::atan2f(this->y, this->x);
        }

        [[nodiscard]]
        constexpr f32 angle_to(const vector2<T>& pt) const
        {
            return (pt - *this).angle();
        }

        [[nodiscard]]
        constexpr f32 dot_product(const vector2<T>& other) const
        {
            return (this->x * other.x) + (this->y * other.y);
        }

        [[nodiscard]]
        constexpr f32 cross_product(const vector2<T> other) const
        {
            return (this->x * other.y) - (this->y * other.x);
        }

        [[nodiscard]]
        constexpr vector2 rotated(const f32 radians) const
        {
            f32 s{ std::sin(radians) };
            f32 c{ std::cos(radians) };

            return vector2{
                (this->x * c) - (this->y * s),
                (this->x * s) + (this->y * c),
            };
        }

        [[nodiscard]]
        constexpr vector2 clamp(vector2<T> min, vector2<T> max) const
        {
            return vector2{
                std::clamp(this->x, min.x, max.x),
                std::clamp(this->y, min.y, max.y),
            };
        }

        [[nodiscard]]
        constexpr vector2<T> lerp(const vector2<T> to, const f32 weight) const
        {
            vector2 ret{ *this };
            ret.x = std::lerp(ret.x, to.x, weight);
            ret.y = std::lerp(ret.y, to.y, weight);
            return ret;
        }

        [[nodiscard]]
        constexpr vector2<T> slerp(const vector2<T> to, const f32 weight) const
        {
            const f32 start_len_sq{ this->length_squared() };
            const f32 end_len_sq{ to.length_squared() };

            if (math::equal(start_len_sq, static_cast<T>(0))
                || math::equal(end_len_sq, static_cast<T>(0))) [[unlikely]] {
                // zero length vectors have no angle, so the best
                // we can do is either lerp or throw an error.
                return this->lerp(to, weight);
            }

            const f32 start_length{ std::sqrt(start_len_sq) };
            const f32 result_length{ std::lerp(start_length, std::sqrt(end_len_sq), weight) };
            const f32 angle{ this->angle_to(to) };

            return this->rotated(angle * weight) * (result_length / start_length);
        }

        [[nodiscard]]
        constexpr vector2<T> move_towards(const vector2<T> target, const f32 delta) const
        {
            const vector2 vec_delta{ target - *this };
            const f32 vd_len{ vec_delta.length() };
            return vd_len <= delta || vd_len < std::numeric_limits<f32>::epsilon()
                     ? target
                     : (*this + vec_delta) / (vd_len * delta);
        }

        [[nodiscard]]
        constexpr vector2<T> slide(const vector2<T> normal) const
        {
            return *this - (normal * this->dot_product(normal));
        }

        [[nodiscard]]
        constexpr vector2<T> reflect(const vector2<T> normal) const
        {
            return (cast::to<T>(2.0) * normal * this->dot_product(normal)) - *this;
        }

        [[nodiscard]]
        constexpr vector2<T> bounce(const vector2<T> normal) const
        {
            return -this->reflect(normal);
        }

    public:
        [[nodiscard]] constexpr bool operator==(vector2<T> other) const
        {
            return math::equal(this->x, other.x) && math::equal(this->y, other.y);
        }

        [[nodiscard]] constexpr bool operator!=(vector2<T> other) const
        {
            return math::not_equal(this->x, other.x) || math::not_equal(this->y, other.y);
        }

        template <rl::floating_point F>
        explicit operator vector2<F>()
            requires rl::integer<T>
        {
            return vector2<F>{
                static_cast<F>(this->x),
                static_cast<F>(this->y),
            };
        }

        [[nodiscard]] constexpr vector2<T> operator-() const noexcept
        {
            return vector2{
                -this->x,
                -this->y,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator+(T other) const noexcept
        {
            return vector2{
                this->x + other,
                this->y + other,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator-(T other) const noexcept
        {
            return vector2{
                this->x - other,
                this->y - other,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator*(T other) const noexcept
        {
            return vector2{
                this->x * other,
                this->y * other,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator/(T other) const noexcept
        {
            return vector2{
                this->x / other,
                this->y / other,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator+(vector2<T> other) const noexcept
        {
            return vector2{
                this->x + other.x,
                this->y + other.y,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator-(vector2<T> other) const noexcept
        {
            return vector2{
                this->x - other.x,
                this->y - other.y,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator*(vector2<T> other) const noexcept
        {
            return vector2{
                this->x * other.x,
                this->y * other.y,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator/(vector2<T> other) const noexcept
        {
            return vector2{
                this->x / other.x,
                this->y / other.y,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator+(dims<T> other) const noexcept
        {
            return vector2{
                static_cast<T>(this->x + other.width),
                static_cast<T>(this->y + other.height),
            };
        }

        [[nodiscard]] constexpr vector2<T> operator-(dims<T> other) const noexcept
        {
            return vector2{
                this->x - other.width,
                this->y - other.height,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator*(dims<T> other) const noexcept
        {
            return vector2{
                this->x * other.width,
                this->y * other.height,
            };
        }

        [[nodiscard]] constexpr vector2<T> operator/(dims<T> other) const noexcept
        {
            return vector2{
                this->x / other.width,
                this->y / other.height,
            };
        }

        constexpr vector2<T>& operator+=(T val) noexcept
        {
            this->x += val;
            this->y += val;
            return *this;
        }

        constexpr vector2<T>& operator-=(T val) noexcept
        {
            this->x -= val;
            this->y -= val;
            return *this;
        }

        constexpr vector2<T>& operator*=(T val) noexcept
        {
            this->x *= val;
            this->y *= val;
            return *this;
        }

        constexpr vector2<T>& operator/=(T val) noexcept
        {
            this->x /= val;
            this->y /= val;
            return *this;
        }

        constexpr vector2<T> operator+=(vector2<T> other) noexcept
        {
            this->x += other.x;
            this->y += other.y;
            return *this;
        }

        constexpr vector2<T> operator-=(vector2<T> other) noexcept
        {
            this->x -= other.x;
            this->y -= other.y;
            return *this;
        }

        constexpr vector2<T> operator*=(vector2<T> other) noexcept
        {
            this->x *= other.x;
            this->y *= other.y;
            return *this;
        }

        constexpr vector2<T>& operator/=(vector2<T> other) noexcept
        {
            this->x /= other.x;
            this->y /= other.y;
            return *this;
        }

        constexpr vector2<T>& operator+=(dims<T> other) noexcept
        {
            this->x += other.width;
            this->y += other.height;
            return *this;
        }

        constexpr vector2<T>& operator-=(dims<T> other) noexcept
        {
            this->x -= other.width;
            this->y -= other.height;
            return *this;
        }

        constexpr vector2<T>& operator*=(dims<T> other) noexcept
        {
            this->x *= other.width;
            this->y *= other.height;
            return *this;
        }

        constexpr vector2<T>& operator/=(dims<T> other) noexcept
        {
            this->x /= other.width;
            this->y /= other.height;
            return *this;
        }

    public:
        T x{ static_cast<T>(0) };
        T y{ static_cast<T>(0) };
        T z{ static_cast<T>(0) };
    };

#pragma pack()
}

namespace rl::ds {
    template <rl::numeric T>
    constexpr auto format_as(const vector2<T>& vec)
    {
        return fmt::format("(x={}, y={})", vec.x, vec.y);
    }
}
