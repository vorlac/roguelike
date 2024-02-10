#pragma once

#include <cmath>
#include <limits>
#include <numbers>
#include <random>
#include <vector>

#include "utils/concepts.hpp"

namespace rl {
    template <auto RangeStart = 0, auto RangeEnd = std::numeric_limits<decltype(RangeStart)>::max()>
        requires(std::same_as<decltype(RangeStart), decltype(RangeEnd)> &&
                 std::integral<decltype(RangeStart)>)
    struct random
    {
    public:
        constexpr static auto value()
        {
            return m_dist(m_engine);
        }

    private:
        constexpr static bool seed()
        {
            m_engine.seed(m_random_device());
            return true;
        }

    private:
        using type = decltype(RangeStart);
        static inline std::random_device m_random_device{};
        static inline std::mt19937 m_engine{};
        static inline std::uniform_int_distribution<random::type> m_dist{ RangeStart, RangeEnd };
        static inline bool m_seeded{ rl::random<RangeStart, RangeEnd>::seed() };
    };
}
