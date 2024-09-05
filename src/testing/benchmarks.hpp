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

namespace rl::bench {
    namespace fib {
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

        rand_benchmarks.run("std::mt19937", [&] {
            const auto rand_val{ random<1, 10000, std::mt19937>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("std::mt19937_64", [&] {
            const auto randval{ random<1, 10000, std::mt19937_64>::value() };
            ankerl::nanobench::doNotOptimizeAway(randval);
        });
        rand_benchmarks.run("std::ranlux24_base", [&] {
            auto randval{ random<1, 10000, std::ranlux24_base>::value() };
            ankerl::nanobench::doNotOptimizeAway(randval);
        });
        rand_benchmarks.run("std::ranlux48_base", [&] {
            auto rand_val{ random<1, 10000, std::ranlux48_base>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("std::ranlux24", [&] {
            auto randval{ random<1, 10000, std::ranlux24>::value() };
            ankerl::nanobench::doNotOptimizeAway(randval);
        });
        rand_benchmarks.run("std::ranlux48;", [&] {
            auto randval{ random<1, 10000, std::ranlux48>::value() };
            ankerl::nanobench::doNotOptimizeAway(randval);
        });
        rand_benchmarks.run("std::knuth_b", [&] {
            const auto rand_val{ random<1, 10000, std::knuth_b>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("std::minstd_rand0", [&] {
            auto rand_val{ random<1, 10000, std::minstd_rand0>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("std::minstd_rand", [&] {
            auto rand_val{ random<1, 10000, std::minstd_rand>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32 / setseq_xsh_rr_64_32", [&] {
            const auto rand_val{ random<1, 10000, pcg32>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_oneseq / oneseq_xsh_rr_64_32", [&] {
            const auto rand_val{ random<1, 10000, pcg32_oneseq>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_unique / unique_xsh_rr_64_32", [&] {
            const auto rand_val{ random<1, 10000, pcg32_unique>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_fast / mcg_xsh_rs_64_32", [&] {
            const auto rand_val{ random<1, 10000, pcg32_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64 / setseq_xsl_rr_128_64", [&] {
            const auto rand_val{ random<1, 10000, pcg64>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_oneseq / oneseq_xsl_rr_128_64", [&] {
            const auto rand_val{ random<1, 10000, pcg64_oneseq>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_unique/unique_xsl_rr_128_64", [&] {
            const auto rand_val{ random<1, 10000, pcg64_unique>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_fast / mcg_xsl_rr_128_64", [&] {
            const auto rand_val{ random<1, 10000, pcg64_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        // rand_benchmarks.run("pcg8_once_insecure / setseq_rxs_m_xs_8_8", [&] {
        //     const auto rand_val{ random<1, 10000, pcg8_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg16_once_insecure / setseq_rxs_m_xs_16_16", [&] {
        //     const auto rand_val{ random<1, 10000, pcg16_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        rand_benchmarks.run("pcg32_once_insecure / setseq_rxs_m_xs_32_32", [&] {
            const auto rand_val{ random<1, 10000, pcg32_once_insecure>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_once_insecure / setseq_rxs_m_xs_64_64", [&] {
            const auto rand_val{ random<1, 10000, pcg64_once_insecure>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        // rand_benchmarks.run("pcg128_once_insecure/setseq_xsl_rr_rr_128_128", [&] {
        //     const auto rand_val{ random<1, 10000, pcg128_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg8_oneseq_once_insecure / oneseq_rxs_m_xs_8_8", [&] {
        //     const auto rand_val{ random<1, 10000, pcg8_oneseq_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg16_oneseq_once_insecure / oneseq_rxs_m_xs_16_16", [&] {
        //     const auto rand_val{ random<1, 10000, pcg16_oneseq_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        rand_benchmarks.run("pcg32_oneseq_once_insecure / oneseq_rxs_m_xs_32_32", [&] {
            const auto rand_val{ random<1, 10000, pcg32_oneseq_once_insecure>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg64_oneseq_once_insecure / oneseq_rxs_m_xs_64_64", [&] {
            const auto rand_val{ random<1, 10000, pcg64_oneseq_once_insecure>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        // rand_benchmarks.run("pcg128_oneseq_once_insecure/oneseq_xsl_rr_rr_128_128", [&] {
        //     const auto rand_val{ random<1, 10000, pcg128_oneseq_once_insecure>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        rand_benchmarks.run("pcg32_k2 / ext_setseq_xsh_rr_64_32<1,16,true>", [&] {
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
        // rand_benchmarks.run("pcg64_k32 / ext_setseq_xsl_rr_128_64<5,16,true>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_k32>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_k32_oneseq / ext_oneseq_xsl_rr_128_64<5,128,true>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_k32_oneseq>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_k32_fast / ext_mcg_xsl_rr_128_64<5,128,true>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_k32_fast>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_c32 / ext_setseq_xsl_rr_128_64<5,16,false>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_c32>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_c32_oneseq / ext_oneseq_xsl_rr_128_64<5,128,false>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_c32_oneseq>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_c32_fast / ext_mcg_xsl_rr_128_64<5,128,false>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_c32_fast>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
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
        // rand_benchmarks.run("pcg64_k1024 / ext_setseq_xsl_rr_128_64<10, 16, true>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_k1024>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_k1024_fast / ext_oneseq_xsl_rr_128_64<10, 128, true>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_k1024_fast>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_c1024 / ext_setseq_xsl_rr_128_64<10, 16, false>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_c1024>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        // rand_benchmarks.run("pcg64_c1024_fast / ext_oneseq_xsl_rr_128_64<10, 128, false>", [&] {
        //     const auto rand_val{ random<1, 10000, pcg64_c1024_fast>::value() };
        //     ankerl::nanobench::doNotOptimizeAway(rand_val);
        // });
        rand_benchmarks.run("pcg32_k16384 / ext_setseq_xsh_rr_64_32<14, 16, true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k16384>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
        rand_benchmarks.run("pcg32_k16384_fast / ext_oneseq_xsh_rs_64_32<14, 32, true>", [&] {
            const auto rand_val{ random<1, 10000, pcg32_k16384_fast>::value() };
            ankerl::nanobench::doNotOptimizeAway(rand_val);
        });
    }

    inline void run_memcmp_benchmarks()
    {
        ankerl::nanobench::Bench memcmp_benchmarks{};
        memcmp_benchmarks.title("memcmp version")
            .unit("comparison")
            .warmup(1000)
            .relative(true)
            .performanceCounters(true);

        memcmp_benchmarks.minEpochTime(1s).run("memcmp", [&] {
            const ds::rect<u32> rect1{
                ds::point{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                },
                ds::dims{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                }
            };
            const ds::rect<u32> rect2{
                ds::point{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                },
                ds::dims{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                }
            };
            const i32 result{ memcmp(&rect1, &rect2, sizeof(rect1)) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        memcmp_benchmarks.minEpochTime(1s).run("static_memcmp", [&] {
            const ds::rect<u32> rect1{
                ds::point{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                },
                ds::dims{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                }
            };
            const ds::rect<u32> rect2{
                ds::point{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                },
                ds::dims{
                    random<1, 100>::value(),
                    random<1, 100>::value(),
                }
            };
            const bool result{ memory::static_memcmp(rect1, rect2) };
            ankerl::nanobench::doNotOptimizeAway(result);
        });

        memcmp_benchmarks.minEpochTime(1s).run("operator==", [&] {
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
