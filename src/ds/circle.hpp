#pragma once

#include <memory>
#include <utility>

#include "core/numeric.hpp"
#include "ds/point.hpp"
#include "utils/concepts.hpp"
#include "utils/memory.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct circle
    {
        explicit constexpr circle() = default;

        explicit constexpr circle(const circle<T>& other) noexcept
            : centroid{ other.centroid }
            , radius{ other.radius }
        {
        }

        explicit constexpr circle(circle<T>&& other) noexcept
            : centroid{ std::move(other.centroid) }
            , radius{ std::move(other.radius) }
        {
        }

        explicit constexpr circle(const T x, const T y, const T r) noexcept
            : centroid{ x, y }
            , radius{ r }
        {
        }

        explicit constexpr circle(const ds::point<T>& pt, const T r) noexcept
            : centroid{ pt }
            , radius{ r }
        {
        }

        explicit constexpr circle(ds::point<T>&& pt, T&& r) noexcept
            : centroid{ std::forward<ds::point<T>>(pt) }
            , radius{ std::forward<T>(r) }
        {
        }

        constexpr inline bool operator==(const circle<T>& other) noexcept
        {
            return 0 == memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        constexpr inline bool operator!=(const circle<T>& other) noexcept
        {
            return !this->operator==(other);
        }

        constexpr inline circle<T>& operator=(const circle<T>& other) noexcept
        {
            std::memcpy(this, &other, sizeof(*this));
            return *this;
        }

        constexpr inline circle<T>& operator=(circle<T>&& other) noexcept
        {
            std::memmove(this, &other, sizeof(*this));
            return *this;
        }

        constexpr inline bool overlaps(const ds::point<T>& pt) noexcept
        {
            f32 dist_sq = centroid.dist_squared(pt);
            return dist_sq <= radius * radius;
        }

        ds::point<T> centroid{ ds::point<T>::null() };
        T radius{ static_cast<T>(0) };
    };

#pragma pack()
}
