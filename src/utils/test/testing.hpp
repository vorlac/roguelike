#pragma once

#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <nanobench.h>

#include "ds/rect.hpp"
#include "utils/generator.hpp"
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
            auto ints{ std::make_integer_sequence<int, 16>{} };
            constexpr static std::array [[maybe_unused]] int_vals{
                int_array(std::forward<decltype(ints)>(ints))
            };
            constexpr static std::array [[maybe_unused]] fib_vals{
                fib_array(std::forward<decltype(ints)>(ints))
            };

            return 0;
        }
    }

    namespace b {

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
            std::tuple tup{ 3, 4.56 };
            double val = 0.0;
            std::apply(
                [&]<typename... T>(T&&... n) {
                    val = sum(std::forward<T>(n)...);
                },
                tup);

            fmt::print("{}", val);

            //========================================================

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

            return 0;
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
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const ds::rect<i32> rect2{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const i32 result{ memcmp(&rect1, &rect2, sizeof(rect1)) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        rand_benchmarks.minEpochTime(1s).run("static_memcmp", [&] {
            const ds::rect<i32> rect1{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const ds::rect<i32> rect2{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const bool result{ memory::static_memcmp(rect1, rect2) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        rand_benchmarks.minEpochTime(1s).run("operator==", [&] {
            const ds::rect<i32> rect1{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const ds::rect<i32> rect2{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const bool result{ rect1 == rect2 };
            ankerl::nanobench::doNotOptimizeAway(result);
        });
    }

    inline void run_coroutine_generator_benchmarks()
    {
        constexpr auto fibonacci{
            [](u32 count = u32_max) -> generator<u64> {
                u64 a{ 0 };
                u64 b{ 1 };
                u64 prev{};
                u32 iterations{ 0 };
                while (iterations++ < count) {
                    co_yield b;
                    prev = a;
                    a = b;
                    b += prev;
                }
            }
        };
        constexpr auto factorial{
            [](u32 iterations = u32_max) -> generator<u64> {
                u64 num{ 1 };
                u64 count{ 0 };
                while (iterations >= ++count) {
                    co_yield num;
                    num *= num++;
                }
            }
        };
        ankerl::nanobench::Bench bench{};
        bench.title("prototypes")
            .unit("iterations")
            .warmup(1000)
            .relative(true)
            .performanceCounters(true);

        using namespace std::literals;
        bench.minEpochTime(1s).run("fib(1)", [&] {
            for (auto&& i : fibonacci(100))
                ankerl::nanobench::doNotOptimizeAway(i);
        });

        bench.minEpochTime(1s).run("fib(2)", [&] {
            auto fac = factorial(100);
            auto ret = std::vector{ fac... };
            ankerl::nanobench::doNotOptimizeAway(ret);
        });
    }
}

namespace rl::circular_nums {
    std::vector<int> primes(int n)
    {
        std::vector<int> primes;
        for (int i = 2; i < n; i++) {
            bool isPrime = true;
            for (int j = 2; j < i; j++) {
                if (i % j == 0) {
                    isPrime = false;
                    break;
                }
            }
            if (isPrime) {
                primes.push_back(i);
            }
        }
        return primes;
    }

    bool check(std::string_view a, std::string_view b)
    {
        for (int i = 0; i < a.size(); i++) {
            bool found = true;
            for (int j = 0; j < a.size(); j++) {
                if (a[j] != b[(i + j) % a.size()]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return true;
            }
        }
        return false;
    }

    bool isCyclic(long long n)
    {
        int length = static_cast<int>(std::log10(n));
        std::string nStr = std::to_string(n * length);
        for (int i = length - 1; i > 0; i--) {
            std::string a = std::to_string(n * i);
            while (a.size() != nStr.size())
                a = "0" + a;
            // std::cout << nStr.size() - a.size() << " ";
            // auto b = std::string(nStr.size() - a.size(), '0');
            if (!check(nStr, a)) {
                return false;
            }
        }
        return true;
    }

    int solution()
    {
        for (auto prime : primes(22)) {
            long double cyclic = (std::pow((long double)10, prime - 1) - 1) / prime;
            [[maybe_unused]] int length = static_cast<int>(std::log10(cyclic));
            if (std::abs(cyclic - std::round(cyclic)) < 0.001 && isCyclic(static_cast<i64>(std::round(cyclic)))) {
                std::string cyclicStr = std::to_string((long)std::round(cyclic));
                fmt::println("{}", cyclicStr);
            }
        }
        return 0;
    }
}
