#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>

#include <fmt/format.h>
#include <nanobench.h>

#include "ds/rect.hpp"
#include "utils/memory.hpp"
#include "utils/numeric.hpp"
#include "utils/random.hpp"

namespace rl::bench::asdf {
    consteval static int fib(const int n)
    {
        if (n <= 1)
            return n;
        return fib(n - 1) + fib(n - 2);
    }

    template <typename T, T... nums>
    static consteval auto fib_array(std::integer_sequence<T, nums...>&& intseq)
    {
        return std::array{ fib(nums)... };
    }

    template <typename T, T... nums>
    static consteval auto int_array(std::integer_sequence<T, nums...>&& intseq)
    {
        return std::array{ nums... };
    }

    int asdf()
    {
        auto ints{ std::make_integer_sequence<int, 28>{} };
        constexpr std::array int_vals{ int_array(std::forward<decltype(ints)>(ints)) };
        constexpr std::array fib_vals{ fib_array(std::forward<decltype(ints)>(ints)) };
    }
}

namespace rl::bench {
    using namespace std::chrono_literals;

    void run_rand_benchmarks()
    {
        ankerl::nanobench::Bench rand_benchmarks{};
        rand_benchmarks.title("Random Number Generators")
            .unit("value")
            .warmup(1000)
            .relative(true)
            .performanceCounters(true);

        rand_benchmarks.minEpochTime(1s).run("c_rand", [&] {
            const i32 randval{ rl::random<1, 100>::value() };
            ankerl::nanobench::doNotOptimizeAway(randval);
            const bool is_in_range{ randval >= 1 && randval <= 100 };
            ankerl::nanobench::doNotOptimizeAway(is_in_range);
        });

        rand_benchmarks.minEpochTime(1s).run("cpp_rand", [&] {
            const i32 randval{ rl::random<1, 100>::value() };
            ankerl::nanobench::doNotOptimizeAway(randval);
            const bool is_in_range{ randval >= 1 && randval <= 100 };
            ankerl::nanobench::doNotOptimizeAway(is_in_range);
        });
    }

    void run_memcmp_benchmarks()
    {
        ankerl::nanobench::Bench rand_benchmarks{};
        rand_benchmarks.title("memcmp version")
            .unit("comparison")
            .warmup(1000)
            .relative(true)
            .performanceCounters(true);

        rand_benchmarks.minEpochTime(1s).run("memcmp", [&] {
            const ds::rect<i32> rect1{
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
            };
            const ds::rect<i32> rect2{
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
            };
            const i32 result{ memcmp(&rect1, &rect2, sizeof(rect1)) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        rand_benchmarks.minEpochTime(1s).run("static_memcmp", [&] {
            const ds::rect<i32> rect1{
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
            };
            const ds::rect<i32> rect2{
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
            };
            const bool result{ memory::static_memcmp(rect1, rect2) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        rand_benchmarks.minEpochTime(1s).run("operator==", [&] {
            const ds::rect<i32> rect1{
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
            };
            const ds::rect<i32> rect2{
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
                rl::random<1, 100>::value(),
            };
            const bool result{ rect1 == rect2 };
            ankerl::nanobench::doNotOptimizeAway(result);
        });
    }
}
