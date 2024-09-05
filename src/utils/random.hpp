#pragma once

#include <concepts>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>

#include <pcg_extras.hpp>
#include <pcg_random.hpp>

#include "utils/conversions.hpp"

namespace rl {
    template <auto RangeStart = 0, auto RangeEnd = std::numeric_limits<decltype(RangeStart)>::max(), typename TRandEngine = std::mt19937>
        requires(std::same_as<decltype(RangeStart), decltype(RangeEnd)> && std::integral<decltype(RangeStart)>)
    struct random
    {
        using numeric_type = decltype(RangeStart);
        using result_type = typename std::type_identity<TRandEngine>::type::result_type;
        using internal_t = typename std::conditional_t<sizeof(result_type) >= sizeof(i32), result_type, i32>;

    public:
        static auto value()
        {
            return m_dist(m_engine);
        }

    private:
        static thread_local inline pcg_extras::seed_seq_from<std::random_device> m_random_device{};
        static thread_local inline TRandEngine m_engine{ m_random_device };
        static thread_local inline std::uniform_int_distribution<internal_t> m_dist{
            static_cast<internal_t>(RangeStart),
            static_cast<internal_t>(RangeEnd),
        };
    };
}
