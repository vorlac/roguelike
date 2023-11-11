#pragma once

#include <cstddef>

#include "core/numeric_types.hpp"

namespace rl::memory
{
    template <std::size_t SIZE>
        requires(SIZE < 32)
    inline constexpr i32 static_memcmp(const void* a, const void* b)
    {
        const auto s1 = static_cast<const i8*>(a);
        const auto s2 = static_cast<const i8*>(b);
        const auto diff = *s1 - *s2;
        return diff ? diff : static_memcmp<SIZE - 1>(s1 + 1, s2 + 1);
    }

    template <>
    inline constexpr i32 static_memcmp<0>(const void*, const void*)
    {
        return 0;
    }
}
