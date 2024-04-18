#pragma once

#include <concepts>
#include <limits>
#include <random>

namespace rl {

    template <auto RangeStart = 0, auto RangeEnd = std::numeric_limits<decltype(RangeStart)>::max()>
        requires(std::same_as<decltype(RangeStart), decltype(RangeEnd)>
                 && std::integral<decltype(RangeStart)>)
    struct random
    {
        using numeric_type = decltype(RangeStart);

    public:
        constexpr static auto value()
            requires std::integral<numeric_type>
        {
            return m_dist(m_engine);
        }

        constexpr static decltype(auto) value()
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
        static inline std::random_device m_random_device{};
        static inline std::mt19937 m_engine{};
        static inline std::uniform_int_distribution<numeric_type> m_dist{ RangeStart, RangeEnd };
        static inline bool m_seeded{ rl::random<RangeStart, RangeEnd>::seed() };
    };

}
