#pragma once

#include <cstdio>
#include <iostream>

#ifdef NDEBUG
  // In release mode the macro does nothing ((void)0), including
  // the execution of the condition so don't define the expression
  // as anything that would be considered program logis.
  #define runtime_assert(condition, message) static_cast<void>(0)
  #define assert_msg(message)                static_cast<void>(0)

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
                        << "  Message  = " << message << std::endl,                \
                  __debugbreak(), 0;                                               \
          }                                                                        \
      }                                                                            \
      while (0)

  #define assert_msg(message) runtime_assert(false, message)

#endif
