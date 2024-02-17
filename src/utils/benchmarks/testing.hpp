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
    namespace a {
        consteval static inline int fib(const int n)
        {
            if (n <= 1)
                return n;
            return fib(n - 1) + fib(n - 2);
        }

        template <typename T, T... nums>
        static inline consteval auto fib_array(std::integer_sequence<T, nums...>&& intseq)
        {
            return std::array{ fib(nums)... };
        }

        template <typename T, T... nums>
        static inline consteval auto int_array(std::integer_sequence<T, nums...>&& intseq)
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

    namespace b {
        /////////////////////////////////////////
        // template <typename... TArgs>
        // auto sum(TArgs&&... args)
        //{
        //     return (args + ...);
        // }
        //
        // int main()
        //{
        //     std::tuple tup{ 3, 4.56 };
        //     double val = 0.0;
        //     std::apply(
        //         [&]<typename... T>(T&&... n) {
        //             val = sum(std::forward<T>(n)...);
        //         },
        //         tup);
        //
        //     fmt::print("{}", val);
        // }
        /////////////////////////////////////////

#include <array>
#include <chrono>
#include <tuple>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

        template <typename... TDurations>
        std::tuple<TDurations...> convert_durations(auto in_duration)
        {
            std::tuple<TDurations...> ret{};

            ((std::get<TDurations>(ret) = std::chrono::duration_cast<TDurations>(in_duration),
              in_duration -= std::chrono::duration_cast<TDurations>(std::get<TDurations>(ret))),
             ...);

            return ret;
        }

        template <typename... TArgs>
        auto sum(TArgs&&... args)
        {
            return (args + ...);
        }

        int main()
        {
            using namespace std::chrono_literals;
            // start with some time like we did in the example above...
            auto orig_duration{ std::chrono::seconds(8742054346) };
            // print out the base time in seconds...
            fmt::print("1 >> orig_duration = {}\n", orig_duration);

            // convert the original time from seconds -> (hours + seconds(
            auto hours_and_mins{ convert_durations<std::chrono::hours, std::chrono::seconds>(
                orig_duration) };

            // std::apply never really gets any easer to look at. the one thing to know about this
            // function is that it allows for you to iterate through the values in a tuple easily.
            // the difficulty with tuples is that every element could be a different type, so in a
            // statically typed langugae like C++, that just means it'll probably always be a bit
            // or a pain to work with them if you need to iterate over the elements in a generic way

            // all this is actually doing is printing out:
            // "2 >> typename of one duration element from the tuple = the duration value from the
            // tuple"
            std::apply(
                [&]<typename... T>(T&&... dur) {
                    (..., fmt::print("2 >> {} = {}\n", typeid(T).name(), std::forward<T>(dur)));
                },
                hours_and_mins);

            // convert the original duration of seconds into hours
            std::tuple orig_to_hours = convert_durations<std::chrono::hours>(orig_duration);

            // print how many hours the original duration is in hours
            fmt::print("3 >> orig_to_hours = {}\n", std::get<0>(orig_to_hours));

            // this std::apply (and the sum function defined above) just takes all of the values in
            // the (hours, sec) tuple we created earlier, converts each of them to hours, then
            // collects the sum of them all (since we want to make sure it matches up with the value
            // we just printed
            std::chrono::hours new_to_hours = 0h;
            std::apply(
                [&]<typename... T>(T&&... n) {
                    new_to_hours = sum(
                        std::chrono::duration_cast<std::chrono::hours>(std::forward<T>(n))...);
                },
                hours_and_mins);

            // and finally print how many hours total we summed up from the values in the tuple to
            // confirm they match the direct sec -> hours conversion we did above. luckily they do
            // haha.
            fmt::print("4 >> new_to_hours = {}\n", new_to_hours);

            // This is what the output of this code looks like:
            //
            // 1 >> orig_duration = 8742054346s
            // 2 >> class std::chrono::duration<int,struct std::ratio<3600,1> > = 2428348h
            // 2 >> class std::chrono::duration<__int64,struct std::ratio<1,1> > = 1546s
            // 3 >> orig_to_hours = 2428348h
            // 4 >> new_to_hours = 2428348h
        }

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
