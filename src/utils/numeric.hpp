#pragma once

#include <cstdint>

#define fast_floating_pnt_mode   0
#define strict_floating_pnt_mode 0

namespace rl {

    using f32 = float;
    using f64 = double;

    using i8 = ::int8_t;
    using i16 = ::int16_t;
    using i32 = ::int32_t;
    using i64 = ::int64_t;

    using i8_fast = ::int_fast8_t;
    using i16_fast = ::int_fast16_t;
    using i32_fast = ::int_fast32_t;
    using i64_fast = ::int_fast64_t;

    using u8 = ::uint8_t;
    using u16 = ::uint16_t;
    using u32 = ::uint32_t;
    using u64 = ::uint64_t;

    using u8_fast = ::uint_fast8_t;
    using u16_fast = ::uint_fast16_t;
    using u32_fast = ::uint_fast32_t;
    using u64_fast = ::uint_fast64_t;

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
