#pragma once

#include <cstddef>

#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

struct small_trivially_copyable
{
    int32_t a{};
    uint64_t b{};
    float c{};
};

namespace rl::memory {
    namespace detail {

        template <std::size_t Size>
            requires(Size < 32)
        constexpr int32_t static_memcmp(const void* a, const void* b)
        {
            const uint8_t* s1{ static_cast<const uint8_t*>(a) };
            const uint8_t* s2{ static_cast<const uint8_t*>(b) };
            const uint8_t diff = s1[0] - s2[0];
            return diff ? diff : detail::static_memcmp<Size - 1>(s1 + 1, s2 + 1);
        }

        template <>
        constexpr int32_t static_memcmp<0>(const void*, const void*)
        {
            return 0;
        }

    }

    template <typename T>
    constexpr bool static_memcmp(const T& a, const T& b)
    {
        constexpr u32 size{ sizeof(T) };
        return 0 == detail::static_memcmp<size>(static_cast<const void*>(&a), static_cast<const void*>(&b));
    }

    template <rl::integer TInt>
    TInt align_to(TInt value, TInt alignment)
    {
        const TInt mod{ value % alignment };
        return (mod != 0)
                 ? value + (alignment - mod)
                 : value;
    }
}

// int main()
//{
//     const small_trivially_copyable a = {
//         .a = static_cast<int32_t>(rand()),
//         .b = static_cast<uint64_t>(rand()),
//         .c = static_cast<float>(rand()) / 10.0f,
//     };
//
//     auto&& static_memcmp_bench = ankerl::nanobench::Bench().run("static_memcmp", [&] {
//         const small_trivially_copyable b = {
//             .a = static_cast<int32_t>(rand()),
//             .b = static_cast<uint64_t>(rand()),
//             .c = static_cast<float>(rand()) / 10.0f,
//         };
//         static_memcmp(a, b);
//         ankerl::nanobench::doNotOptimizeAway(a);
//     });
//
//     auto&& std_memcmp_bench = ankerl::nanobench::Bench().run("memcmp", [&] {
//         const small_trivially_copyable b = {
//             .a = static_cast<int32_t>(rand()),
//             .b = static_cast<uint64_t>(rand()),
//             .c = static_cast<float>(rand()) / 10.0f,
//         };
//         (void)memcmp(&a, &b, sizeof(a));
//         ankerl::nanobench::doNotOptimizeAway(a);
//     });
//
//     std::cout << static_memcmp_bench.output();
//     std::cout << std::endl << std::endl;
//     std::cout << std_memcmp_bench.output();
//     std::cout << std::endl << std::endl;
// }
