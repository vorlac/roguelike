#pragma once

#include <cmath>

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
    struct dims {
    public:
        consteval dims() = default;

        constexpr dims(T w, T h)
            : width{ w }
            , height{ h } {
        }

        explicit constexpr dims(margin<T> m)
            : width{ m.horizontal() }
            , height{ m.vertical() } {
        }

        [[nodiscard]]
        consteval static dims<T> null() {
            return dims<T>{ -1, -1 };
        }

        [[nodiscard]]
        consteval static dims<T> zero() {
            return dims<T>{ 0, 0 };
        }

        [[nodiscard]]
        constexpr bool is_null() const {
            return *this == dims<T>::null();
        }

        [[nodiscard]]
        constexpr bool empty() const {
            return *this == dims<T>::zero();
        }

        [[nodiscard]]
        constexpr bool valid() const {
            return this->width > 0 &&
                   this->height > 0;
        }

        [[nodiscard]]
        constexpr bool invalid() const {
            return !this->valid();
        }

        [[nodiscard]]
        constexpr T area() const {
            return this->width * this->height;
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr operator dims<I>() const {
            return dims<I>{
                static_cast<I>(std::round(this->width)),
                static_cast<I>(std::round(this->height)),
            };
        }

        template <rl::floating_point F>
            requires rl::integer<T>
        constexpr operator dims<F>() const {
            return dims<F>{
                static_cast<F>(this->width),
                static_cast<F>(this->height),
            };
        }

        [[nodiscard]]
        dims<T> merged(dims<T> other) const {
            dims ret{ *this };
            if (math::equal(this->width, 0.0f))
                ret.width = other.width;
            if (math::equal(this->height, 0.0f))
                ret.height = other.height;
            return ret;
        }

        template <rl::floating_point F>
            requires rl::integer<T>
        constexpr bool operator==(const dims<F> other) const {
            return math::equal(this->height, std::round(other.height)) &&
                   math::equal(this->width, std::round(other.width));
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr bool operator==(const dims<I> other) const {
            return math::equal(std::lroundf(this->height), other.height) &&
                   math::equal(std::lroundf(this->width), other.width);
        }

        constexpr bool operator==(const dims<T> other) const {
            return math::equal(this->height, other.height) &&
                   math::equal(this->width, other.width);
        }

        template <rl::floating_point F>
            requires rl::integer<T>
        constexpr bool operator!=(const dims<F> other) const {
            return math::not_equal(this->height, static_cast<T>(std::lroundf(other.height))) &&
                   math::not_equal(this->width, static_cast<T>(std::lroundf(other.width)));
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr bool operator!=(const dims<I> other) const {
            return math::not_equal(std::lroundf(this->height), other.height) &&
                   math::not_equal(std::lroundf(this->width), other.width);
        }

        constexpr bool operator!=(const dims<T> other) const {
            return !this->operator==(other);
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator+(V val) const {
            return dims{
                static_cast<T>(this->width + val),
                static_cast<T>(this->height + val),
            };
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator-(V val) const {
            return dims{
                static_cast<T>(this->width - val),
                static_cast<T>(this->height - val),
            };
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator*(V val) const {
            return dims{
                static_cast<T>(this->width * val),
                static_cast<T>(this->height * val),
            };
        }

        template <rl::numeric V>
            requires rl::floating_point<T> || rl::integer<T, V>
        constexpr dims<T> operator/(V val) const {
            return dims{
                static_cast<T>(this->width / val),
                static_cast<T>(this->height / val),
            };
        }

        constexpr dims<T> operator+(dims<T> other) const {
            return dims{
                static_cast<T>(this->width + other.width),
                static_cast<T>(this->height + other.height),
            };
        }

        constexpr dims<T> operator-(dims<T> other) const {
            return dims{
                this->width - other.width,
                this->height - other.height,
            };
        }

        constexpr dims<T> operator*(dims<T> other) const {
            return dims{
                this->width * other.width,
                this->height * other.height,
            };
        }

        constexpr dims<T> operator/(dims<T> other) const {
            return dims{
                this->width / other.width,
                this->height / other.height,
            };
        }

        constexpr dims<T> operator+(vector2<T> other) const {
            return dims{
                this->width + other.x,
                this->height + other.y,
            };
        }

        constexpr dims<T> operator-(vector2<T> other) const {
            return dims{
                this->width - other.x,
                this->height - other.y,
            };
        }

        constexpr dims<T> operator*(vector2<T> other) const {
            return dims{
                this->width * other.x,
                this->height * other.y,
            };
        }

        constexpr dims<T> operator/(vector2<T> other) const {
            return dims{
                this->width / other.x,
                this->height / other.y,
            };
        }

        constexpr dims<T> operator+(margin<T> other) const {
            return dims{
                this->width + other.horizontal(),
                this->height + other.vertical(),
            };
        }

        constexpr dims<T> operator-(margin<T> other) const {
            return dims{
                this->width - other.horizontal(),
                this->height - other.vertical(),
            };
        }

        constexpr dims<T>& operator+=(T val) {
            this->width += val;
            this->height += val;
            return *this;
        }

        constexpr dims<T>& operator-=(T val) {
            this->width -= val;
            this->height -= val;
            return *this;
        }

        constexpr dims<T>& operator*=(T val) {
            this->width *= val;
            this->height *= val;
            return *this;
        }

        constexpr dims<T>& operator/=(T val) {
            this->width /= val;
            this->height /= val;
            return *this;
        }

        constexpr dims<T>& operator+=(dims<T> other) {
            this->width += other.width;
            this->height += other.height;
            return *this;
        }

        constexpr dims<T>& operator-=(dims<T> other) {
            this->width -= other.width;
            this->height -= other.height;
            return *this;
        }

        constexpr dims<T>& operator*=(dims<T> other) {
            this->width *= other.width;
            this->height *= other.height;
            return *this;
        }

        constexpr dims<T>& operator/=(dims<T> other) {
            this->width /= other.width;
            this->height /= other.height;
            return *this;
        }

        constexpr dims<T>& operator+=(margin<T> other) {
            this->width += other.horizontal();
            this->height += other.vertical();
            return *this;
        }

        constexpr dims<T>& operator-=(margin<T> other) {
            this->width -= other.horizontal();
            this->height -= other.vertical();
            return *this;
        }

        constexpr dims<T>& operator+=(vector2<T> other) {
            this->width += other.x;
            this->height += other.y;
            return *this;
        }

        constexpr dims<T>& operator-=(vector2<T> other) {
            this->width -= other.x;
            this->height -= other.y;
            return *this;
        }

        constexpr dims<T>& operator*=(vector2<T> other) {
            this->width *= other.x;
            this->height *= other.y;
            return *this;
        }

        constexpr dims<T>& operator/=(vector2<T> other) {
            this->width /= other.x;
            this->height /= other.y;
            return *this;
        }

    public:
        T width{ 0 };
        T height{ 0 };
    };

#pragma pack()
}

namespace rl::ds {
    template <rl::numeric T>
    constexpr auto format_as(dims<T> size) {
        return fmt::format("(w={}, h={})", size.width, size.height);
    }
}
