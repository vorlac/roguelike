#pragma once

#include <cstdint>

namespace rl
{
    using f32 = std::float_t;
    using f64 = std::double_t;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using i8_fast  = std::int_fast8_t;
    using i16_fast = std::int_fast16_t;
    using i32_fast = std::int_fast32_t;
    using i64_fast = std::int_fast64_t;

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using u8_fast  = std::uint_fast8_t;
    using u16_fast = std::uint_fast16_t;
    using u32_fast = std::uint_fast32_t;
    using u64_fast = std::uint_fast64_t;

    using sizetype = std::size_t;
    using ptrdiff  = std::ptrdiff_t;
    using ptrsize  = std::intptr_t;

    static inline constexpr u32 ptr_byte_size{ sizeof(void*) };
}
