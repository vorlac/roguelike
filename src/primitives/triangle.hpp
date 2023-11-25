#pragma once

#include <array>
#include <utility>

#include "primitives/point.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
    template <rl::numeric T>
    class triangle
    {
    public:
        constexpr inline triangle(const T a, const T b, const T c)
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

    private:
        std::array<ds::point<T>, 3> m_points{};
    };

}
