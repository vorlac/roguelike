#pragma once

#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <nanobench.h>
#include <pcg_random.hpp>

#include "ds/rect.hpp"
#include "utils/generator.hpp"
#include "utils/memory.hpp"
#include "utils/numeric.hpp"
#include "utils/random.hpp"
#include "utils/test/input.hpp"

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
            [[maybe_unused]] constexpr static std::array int_vals{
                int_array(std::forward<decltype(ints)>(ints))
            };
            [[maybe_unused]] constexpr static std::array fib_vals{
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
    using ll = int;  // long long;

    class OrigRecursiveSolution
    {
    public:
        long long maximumTotalDamage(std::vector<int>& power)
        {
            std::unordered_map<ll, ll> freq;
            for (int x : power)
                freq[x]++;

            std::sort(power.begin(), power.end());
            power.erase(std::unique(power.begin(), power.end()), power.end());
            int n = static_cast<int>(power.size());

            std::vector<ll> memo(n, -1);
            auto max_damage = [&](auto&& self, int i) -> ll {
                if (i < 0)
                    return 0;

                if (memo[i] != -1)
                    return memo[i];

                ll skip = self(self, i - 1);
                int j = i - 1;
                while (j >= 0 && power[j] >= power[i] - 2)
                    j--;
                ll take = power[i] * freq[power[i]] + self(self, j);
                return memo[i] = std::max(skip, take);
            };

            return max_damage(max_damage, n - 1);
        }
    };

    class OrigIterativeSolution
    {
    public:
        long long maximumTotalDamage(std::vector<int>& power)
        {
            std::unordered_map<ll, ll> freq;
            for (int x : power)
                freq[x]++;

            std::sort(power.begin(), power.end());
            power.erase(std::unique(power.begin(), power.end()), power.end());
            int n = static_cast<int>(power.size());

            std::vector<ll> dp(n, 0);
            dp[0] = power[0] * freq[power[0]];
            for (int i = 1; i < n; i++) {
                int j = i - 1;
                while (j >= 0 && power[j] >= power[i] - 2)
                    j--;
                ll take = power[i] * freq[power[i]] + (j >= 0 ? dp[j] : 0LL);
                ll skip = dp[i - 1];
                dp[i] = std::max(take, skip);
            }
            return dp[n - 1];
        }
    };

    class NewRecursiveSolution
    {
    public:
        long long maximumTotalDamage(std::vector<int>& power)
        {
            int run = 0;
            std::unordered_map<ll, ll> freq;
            for (int x : power)
                freq[x]++;

            std::sort(power.begin(), power.end());
            power.erase(std::unique(power.begin(), power.end()), power.end());
            std::vector<ll> wtf2(power.begin(), power.end());
            int n = static_cast<int>(power.size());

            std::vector<ll> memo(n, -1);
            auto max_damage = [&](auto&& self, int i) -> ll {
                if (i < 0)
                    return 0;

                if (memo[i] != -1)
                    return memo[i];

                ll skip = self(self, i - 1);
                int j = i - 1;
                while (j >= 0 && power[j] >= power[i] - 2)
                    j--;
                ll wtf = wtf2[i];
                ++run;
                ll take = wtf * freq[wtf2[i]] + self(self, j);
                return memo[i] = std::max(skip, take);
            };
            return max_damage(max_damage, n - 1);
        }
    };

    inline void run_lc_benchmark()
    {
        OrigRecursiveSolution or_sln{};
        OrigIterativeSolution oi_sln{};
        NewRecursiveSolution nr_sln{};

        using namespace std::literals;
        ankerl::nanobench::Bench lc_benchmarks{};
        lc_benchmarks.title("Random Number Generators")
            .unit("value")
            .warmup(10)
            .relative(true)
            .performanceCounters(true)
            .minEpochTime(250ms);

        lc_benchmarks.minEpochTime(1s).run("OrigRecursiveSolution", [&] {
            auto ret = or_sln.maximumTotalDamage(benchmark::BENCHMARK_INPUT);
            ankerl::nanobench::doNotOptimizeAway(ret);
        });
        lc_benchmarks.minEpochTime(1s).run("OrigIterativeSolution", [&] {
            auto ret = oi_sln.maximumTotalDamage(benchmark::BENCHMARK_INPUT);
            ankerl::nanobench::doNotOptimizeAway(ret);
        });
        lc_benchmarks.minEpochTime(1s).run("NewRecursiveSolution", [&] {
            auto ret = nr_sln.maximumTotalDamage(benchmark::BENCHMARK_INPUT);
            ankerl::nanobench::doNotOptimizeAway(ret);
        });
    }
}

namespace rl::bench {
    using namespace std::chrono_literals;

    inline void run_rand_benchmarks()
    {
        using namespace std::literals;
        ankerl::nanobench::Bench rand_benchmarks{};
        rand_benchmarks.title("Random Number Generators")
            .unit("value")
            .warmup(1000)
            .relative(true)
            .performanceCounters(true)
            .minEpochTime(250ms);

        // rand_benchmarks.run("std::mt19937", [&] {
        //     const auto rand_val{ random<1, 10000, std::mt19937>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("std::mt19937_64", [&] {
        //     const auto randval{ random<1, 10000, std::mt19937_64>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(randval);
        // });
        // rand_benchmarks.run("std::ranlux24_base", [&] {
        //     auto randval{ random<1, 10000, std::ranlux24_base>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(randval);
        // });
        // rand_benchmarks.run("std::ranlux48_base", [&] {
        //     auto rand_val{ random<1, 10000, std::ranlux48_base>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("std::ranlux24", [&] {
        //     auto randval{ random<1, 10000, std::ranlux24>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(randval);
        // });
        // rand_benchmarks.run("std::ranlux48;", [&] {
        //     auto randval{ random<1, 10000, std::ranlux48>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(randval);
        // });
        // rand_benchmarks.run("std::knuth_b", [&] {
        //     const auto rand_val{ random<1, 10000, std::knuth_b>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("std::minstd_rand0", [&] {
        //     auto rand_val{ random<1, 10000, std::minstd_rand0>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("std::minstd_rand", [&] {
        //     auto rand_val{ random<1, 10000, std::minstd_rand>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg32 / setseq_xsh_rr_64_32", [&] {
        //     const auto rand_val{ random<1, 10000, pcg32>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg32_oneseq / oneseq_xsh_rr_64_32", [&] {
        //     const auto rand_val{ random<1, 10000, pcg32_oneseq>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg32_unique / unique_xsh_rr_64_32", [&] {
        //     const auto rand_val{ random<1, 10000, pcg32_unique>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg32_fast / mcg_xsh_rs_64_32", [&] {
        //     const auto rand_val{ random<1, 10000, pcg32_fast>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64 / setseq_xsl_rr_128_64", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_oneseq / oneseq_xsl_rr_128_64", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_oneseq>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_unique/unique_xsl_rr_128_64", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_unique>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_fast / mcg_xsl_rr_128_64", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_fast>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        //  rand_benchmarks.run("pcg8_once_insecure / setseq_rxs_m_xs_8_8", [&] {
        //      const auto rand_val{ random<1, 10000, pcg8_once_insecure>::value() };
        //      ankerl::nanobench::doNotOptimizeAway(rand_val);
        //  });
        // rand_benchmarks.run("pcg16_once_insecure / setseq_rxs_m_xs_16_16", [&] {
        //     const auto rand_val{ random<1, 10000, pcg16_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg32_once_insecure / setseq_rxs_m_xs_32_32", [&] {
        //     const auto rand_val{ random<1, 10000, pcg32_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_once_insecure / setseq_rxs_m_xs_64_64", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        //  rand_benchmarks.run("pcg128_once_insecure/setseq_xsl_rr_rr_128_128", [&] {
        //      const auto rand_val{ random<1, 10000, pcg128_once_insecure>::value() };
        //      ankerl::nanobench::doNotOptimizeAway(rand_val);
        //  });
        /* rand_benchmarks.run("pcg8_oneseq_once_insecure / oneseq_rxs_m_xs_8_8", [&] {
             const auto rand_val{ random<1, 10000, pcg8_oneseq_once_insecure>::value() };
             ankerl::nanobench::doNotOptimizeAway(rand_val);
         });
         rand_benchmarks.run("pcg16_oneseq_once_insecure / oneseq_rxs_m_xs_16_16", [&] {
             const auto rand_val{ random<1, 10000, pcg16_oneseq_once_insecure>::value() };
             ankerl::nanobench::doNotOptimizeAway(rand_val);
         });
         rand_benchmarks.run("pcg32_oneseq_once_insecure / oneseq_rxs_m_xs_32_32", [&] {
             const auto rand_val{ random<1, 10000, pcg32_oneseq_once_insecure>::value() };
             ankerl::nanobench::doNotOptimizeAway(rand_val);
         });
         rand_benchmarks.run("pcg64_oneseq_once_insecure / oneseq_rxs_m_xs_64_64", [&] {
             const auto rand_val{ random<1, 10000, pcg64_oneseq_once_insecure>::value() };
             ankerl::nanobench::doNotOptimizeAway(rand_val);
         });*/
        // rand_benchmarks.run("pcg128_oneseq_once_insecure/oneseq_xsl_rr_rr_128_128", [&] {
        //     const auto rand_val{ random<1, 10000, pcg128_oneseq_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        /*rand_benchmarks.run("pcg32_k2 / ext_setseq_xsh_rr_64_32<1,16,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k2>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k2_fast / ext_oneseq_xsh_rs_64_32<1,32,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k2_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k64 / ext_setseq_xsh_rr_64_32<6,16,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k64>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k64_oneseq / ext_mcg_xsh_rs_64_32<6,32,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k64_oneseq>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k64_fast / ext_oneseq_xsh_rs_64_32<6,32,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k64_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_c64 / ext_setseq_xsh_rr_64_32<6,16,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_c64>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_c64_oneseq / ext_oneseq_xsh_rs_64_32<6,32,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_c64_oneseq>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_c64_fast / ext_mcg_xsh_rs_64_32<6,32,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_c64_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_k32 / ext_setseq_xsl_rr_128_64<5,16,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_k32>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_k32_oneseq / ext_oneseq_xsl_rr_128_64<5,128,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_k32_oneseq>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_k32_fast / ext_mcg_xsl_rr_128_64<5,128,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_k32_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_c32 / ext_setseq_xsl_rr_128_64<5,16,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_c32>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_c32_oneseq / ext_oneseq_xsl_rr_128_64<5,128,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_c32_oneseq>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_c32_fast / ext_mcg_xsl_rr_128_64<5,128,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_c32_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k1024 / ext_setseq_xsh_rr_64_32<10,16,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k1024>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k1024_fast / ext_oneseq_xsh_rs_64_32<10,32,true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k1024_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_c1024 / ext_setseq_xsh_rr_64_32<10,16,false>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_c1024>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_c1024_fas / ext_oneseq_xsh_rs_64_32<10,32,false>t", [&] {
            const auto rand_val{ random<1, 10000, pcg32_c1024_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_k1024 / ext_setseq_xsl_rr_128_64<10, 16, true>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_k1024>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_k1024_fast / ext_oneseq_xsl_rr_128_64<10, 128, true>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_k1024_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_c1024 / ext_setseq_xsl_rr_128_64<10, 16, false>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_c1024>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_c1024_fast / ext_oneseq_xsl_rr_128_64<10, 128, false>", [&] {
            const auto rand_val{ random<1, 10000, pcg64_c1024_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k16384 / ext_setseq_xsh_rr_64_32<14, 16, true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k16384>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k16384_fast / ext_oneseq_xsh_rs_64_32<14, 32, true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k16384_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });*/
    }

    inline void run_memcmp_benchmarks()
    {
        ankerl::nanobench::Bench rand_benchmarks{};
        rand_benchmarks.title("memcmp version")
            .unit("comparison")
            .warmup(1000)
            .relative(true)
            .performanceCounters(true);

        rand_benchmarks.minEpochTime(1s).run("memcmp", [&] {
            const ds::rect<u32> rect1{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const ds::rect<u32> rect2{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const i32 result{ memcmp(&rect1, &rect2, sizeof(rect1)) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        rand_benchmarks.minEpochTime(1s).run("static_memcmp", [&] {
            const ds::rect<u32> rect1{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const ds::rect<u32> rect2{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            // const bool result{ memory::static_memcmp(rect1, rect2) };
            // ankerl::nanobench::doNotOptimizeAway(result);
        });

        rand_benchmarks.minEpochTime(1s).run("operator==", [&] {
            const ds::rect<u32> rect1{
                { random<1, 100>::value(),
                  random<1, 100>::value() },
                { random<1, 100>::value(),
                  random<1, 100>::value() },
            };
            const ds::rect<u32> rect2{
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
            // auto ret = std::vector{ fac... };
            // ankerl::nanobench::doNotOptimizeAway(ret);
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
