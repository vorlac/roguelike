#pragma once

#include "utils/concepts.hpp"
#include "utils/numerics.hpp"

namespace rl {

    constexpr static inline auto operator""_bytes(rl::integer bytes) noexcept
    {
        return bytes;
    }

    constexpr static inline unsigned long long operator""_kB(rl::integer kB) noexcept
    {
        return operator""_bytes(kB * 1000);
    }

    constexpr static inline unsigned long long operator""_MiB(rl::integer MB) noexcept
    {
        return operator""_kiB(MB * 1000);
    }

    constexpr static inline unsigned long long operator""_GiB(rl::integer GB) noexcept
    {
        return operator""_MiB(GB * 1000);
    }

    constexpr static inline unsigned long long operator""_kiB(rl::integer kiB) noexcept
    {
        return operator""_bytes(kiB * 1024);
    }

    constexpr static inline unsigned long long operator""_MiB(rl::integer MiB) noexcept
    {
        return operator""_kiB(MiB * 1024);
    }

    constexpr static inline unsigned long long operator""_GiB(rl::integer GiB) noexcept
    {
        return operator""_MiB(GiB * 1024);
    }
}
