#pragma once

#include <cstdint>

#define fast_floating_pnt_mode   0
#define strict_floating_pnt_mode 0

namespace rl {

    using f32 = float_t;
    using f64 = double_t;

    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using i8_fast = std::int_fast8_t;
    using i16_fast = std::int_fast16_t;
    using i32_fast = std::int_fast32_t;
    using i64_fast = std::int_fast64_t;

    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using u8_fast = std::uint_fast8_t;
    using u16_fast = std::uint_fast16_t;
    using u32_fast = std::uint_fast32_t;
    using u64_fast = std::uint_fast64_t;

#if fast_floating_pnt_mode             // === fast fp ===
  #pragma float_control(except, off)   // disable exception semantics
  #pragma fenv_access(off)             // disable environment sensitivity
  #pragma float_control(precise, off)  // disable precise semantics
  #pragma fp_contract(on)              // enable contractions
#elif strict_floating_pnt_mode         // === strict fp ===
  #pragma float_control(precise, on)   // enable precise semantics
  #pragma fenv_access(on)              // enable environment sensitivity
  #pragma float_control(except, on)    // enable exception semantics
#endif
}
