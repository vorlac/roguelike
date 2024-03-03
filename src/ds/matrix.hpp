#pragma once

#include <array>
#include <utility>

#include <fmt/format.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"

namespace rl::ds {
#pragma pack(4)

    template <u8 N = 3, rl::numeric T>
    struct matrix
    {
    public:
        constexpr matrix()
        {
            for (u32 i = 0; i < N; ++i)
                m_data[(i * N) + i] = 1;
        }

        constexpr ~matrix() = default;

        constexpr matrix(T w, T h)
            : width{ w }
            , height{ h }
        {
        }

    public:
    public:
        std::array<T, N * N> m_data{ 0 };
    };
}
