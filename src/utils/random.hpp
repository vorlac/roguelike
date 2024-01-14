#pragma once

#include <cmath>
#include <limits>
#include <numbers>
#include <random>
#include <vector>

#include "utils/concepts.hpp"

namespace rl {

    template <i32 RangeStart = 0, i32 RangeEnd = std::numeric_limits<i32>::max()>
    struct crand
    {
    public:
        constexpr static inline i32 value()
        {
            return (::rand() % m_range) + RangeStart;
        }

    private:
        constexpr static inline time_t* seed(time_t* seed = nullptr)
        {
            const u32 seed_val{ static_cast<u32>(::time(seed)) };
            ::srand(seed_val);
            return seed;
        }

    private:
        const static inline i32 m_range{ RangeEnd - RangeStart };
        const static inline ::time_t* m_seed{ rl::crand<RangeStart, RangeEnd>::seed() };
    };

    template <i32 RangeStart = 0, i32 RangeEnd = std::numeric_limits<i32>::max()>
    struct random
    {
    public:
        constexpr static inline i32 value()
        {
            return m_uniform_dist(m_engine);
        }

    private:
        constexpr static inline bool seed()
        {
            m_engine.seed(m_random_device());  // Seed random engine
            return true;
        }

    private:
        static inline std::random_device m_random_device{};
        static inline std::mt19937 m_engine{};
        static inline std::uniform_int_distribution<i32> m_uniform_dist{ RangeStart, RangeEnd };
        static inline bool m_seeded{ rl::random<RangeStart, RangeEnd>::seed() };
    };

}
