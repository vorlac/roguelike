#pragma once

#include <cstdio>
#include <iostream>

#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_error.h>
SDL_C_LIB_END

#ifdef NDEBUG
  // In release mode the macro does nothing ((void)0), including
  // the execution of the condition so don't define the expression
  // as anything that would be considered program logis.
  #define runtime_assert(condition, message) static_cast<void>(0)
  #define assert_cond(condition)             static_cast<void>(0)
  #define assert_msg(message)                static_cast<void>(0)
  #define sdl_assert(condition, message)     static_cast<void>(0)

#else

// In debug mode, checks the passed in condition and outputs
// detailed information to stederr, including a custom error
// message when the condition evaluates to false.
  #define runtime_assert(condition, message)                                       \
      do                                                                           \
      {                                                                            \
          if (!(condition)) [[unlikely]]                                           \
          {                                                                        \
              std::cerr << "Assertion failed: (" << #condition << ")" << std::endl \
                        << "  Function = " << __FUNCTION__ << std::endl            \
                        << "  File     = " << __FILE__ << std::endl                \
                        << "  Line     = " << __LINE__ << std::endl                \
                        << "  Message  = " << message << std::endl;                \
              __debugbreak();                                                      \
          }                                                                        \
      }                                                                            \
      while (0)

  #define sdl_assert(condition, message)                                           \
      do                                                                           \
      {                                                                            \
          if (!(condition)) [[unlikely]]                                           \
          {                                                                        \
              char sdl_error[256] = { 0 };                                         \
              SDL3::SDL_GetErrorMsg(sdl_error, 255);                               \
              std::cerr << "Assertion failed: (" << #condition << ")" << std::endl \
                        << "  Function  = " << __FUNCTION__ << std::endl           \
                        << "  File      = " << __FILE__ << std::endl               \
                        << "  Line      = " << __LINE__ << std::endl               \
                        << "  Message   = " << message << std::endl                \
                        << "  SDL Error = " << sdl_error << std::endl;             \
                                                                                   \
              __debugbreak();                                                      \
          }                                                                        \
      }                                                                            \
      while (0)

  #define assert_msg(message) runtime_assert(false, message)
  #define assert_cond(cond)   runtime_assert(cond, "condition check failed")

#endif
