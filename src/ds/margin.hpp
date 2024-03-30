#pragma once

#include <cmath>
#include <memory>
#include <utility>

#include <fmt/format.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/memory.hpp"

namespace rl::ds {
#pragma pack(4)

    // template <rl::numeric T>
    // struct point;

    template <rl::numeric T>
    struct margin
    {
    public:
        [[nodiscard]]
        constexpr T vertical() const
        {
            return static_cast<T>(top + bottom);
        }

        [[nodiscard]]
        constexpr T horizontal() const
        {
            return static_cast<T>(left + right);
        }

        constexpr bool operator==(const margin<T>& other) const
        {
            return math::equal(this->top, other.top) &&        //
                   math::equal(this->bottom, other.bottom) &&  //
                   math::equal(this->left, other.left) &&      //
                   math::equal(this->right, other.right);
        }

        constexpr bool operator!=(const margin<T>& other) const
        {
            return !this->operator==(other);
        }

        constexpr margin<T>& operator/=(const T& val)
        {
            this->top /= val;
            this->bottom /= val;
            this->left /= val;
            this->right /= val;
            return *this;
        }

        constexpr margin<T> operator*(const T& val) const
        {
            margin ret{ *this };
            return ret * val;
        }

        // constexpr margin<T> operator*(const vector2<T>& vec) const
        //{
        //     return margin<T>{
        //         this->top * vec.y,
        //         this->bottom * vec.y,
        //         this->left * vec.x,
        //         this->right * vec.y,
        //     };
        // }

        // constexpr margin<T>& operator*=(const T& val)
        //{
        //     this->top *= val;
        //     this->bottom *= val;
        //     this->left *= val;
        //     this->right *= val;
        //     return *this;
        // }

    public:
        T top{ 0 };
        T bottom{ 0 };
        T left{ 0 };
        T right{ 0 };
    };

    template <rl::numeric T>
    constexpr auto format_as(const margin<T>& mar)
    {
        return fmt::format("margin=[t:{} b:{} l:{} r:{}]", mar.top, mar.bottom, mar.left, mar.right);
    }

#pragma pack()
}
