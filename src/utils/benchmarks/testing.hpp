#pragma once

#include <chrono>
#include <iostream>
#include <string>

#include <fmt/format.h>
#include <nanobench.h>

#include "ds/rect.hpp"
#include "utils/memory.hpp"
#include "utils/numeric.hpp"
#include "utils/random.hpp"

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
            const i32 randval{ rl::crand<1, 100>::value() };
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
