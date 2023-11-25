#pragma once

#include <array>
#include <utility>

#include "primitives/point.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
    template <rl::numeric T>
    struct triangle
    {
        constexpr inline triangle()
            : m_points{
                ds::point<T>{},
                ds::point<T>{},
                ds::point<T>{},
            }
        {
        }

        constexpr inline triangle(const ds::point<T>& a, const ds::point<T>& b,
                                  const ds::point<T>& c)
            : m_points{ a, b, c }
        {
        }

        constexpr inline T a() const
        {
            return m_points[0];
        }

        constexpr inline T b() const
        {
            return m_points[1];
        }

        constexpr inline T c() const
        {
            return m_points[2];
        }

        constexpr inline operator std::array<T, 9>*()
        {
            static_assert(sizeof(m_points) == (sizeof(T) * 9), "inconsistent vertex buffer size");
            return m_points.data();
        }

        constexpr inline operator std::array<T, 9>()
        {
            static_assert(sizeof(m_points) == (sizeof(T) * 9), "inconsistent vertex buffer size");
            return *m_points.data();
        }

    private:
        std::array<ds::point<T>, 3> m_points{};
    };
}
