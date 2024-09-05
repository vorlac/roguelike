#pragma once

#include <cstddef>

#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

namespace rl::memory {
    namespace detail {
        template <u64 Size>
            requires(Size < 32)
        constexpr i32 static_memcmp(const void* a, const void* b)
        {
            const u8* const s1{ static_cast<const u8*>(a) };
            const u8* const s2{ static_cast<const u8*>(b) };
            const u8 diff = s1[0] - s2[0];
            return diff ? diff
                        : detail::static_memcmp<Size - 1>(s1 + 1, s2 + 1);
        }

        template <>
        constexpr i32 static_memcmp<0>(const void*, const void*)
        {
            return 0;
        }

    }

    template <typename T>
    constexpr bool static_memcmp(const T& a, const T& b)
    {
        constexpr static auto size{ sizeof(T) };
        auto ret = detail::static_memcmp<size>(
            static_cast<const void*>(&a),
            static_cast<const void*>(&b));
        return ret == 0;
    }

    template <rl::integer I>
    I align_to(I value, I alignment)
    {
        const I mod{ value % alignment };
        return mod != 0 ? value + (alignment - mod)
                        : value;
    }
}
