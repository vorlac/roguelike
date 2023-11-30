#pragma once

#include <cstdio>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

#include <fmt/color.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

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

constexpr static fmt::text_style ASSERT_COLOR = fmt::fg(fmt::color(0xD4A4A4));

// In debug mode, checks the passed in condition and outputs
// detailed information to stederr, including a custom error
// message when the condition evaluates to false.
  #define runtime_assert(condition, fmtstr, ...)                                                    \
      do                                                                                            \
      {                                                                                             \
          if (!(condition)) [[unlikely]]                                                            \
          {                                                                                         \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                             \
                         fmt::format("Assertion failed: ({}{}\n",                                   \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), #condition),        \
                                     fmt::format(ASSERT_COLOR, ")")));                              \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                             \
                         fmt::format("  Function = {}\n",                                           \
                                     fmt::format(fmt::fg(fmt::color(0xB6ADDB)), __FUNCTION__)));    \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                             \
                         fmt::format("  File     = {}\n",                                           \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), __FILE__)));        \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                             \
                         fmt::format("  Line     = {}\n",                                           \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), "{}", __LINE__)));  \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                             \
                         fmt::format(                                                               \
                             "  Message  = {}",                                                     \
                             fmt::format(fmt::fg(fmt::color(0xCAB880)), "{}",                       \
                                         fmt::join(fmt::format(fmtstr __VA_OPT__(, ) __VA_ARGS__) | \
                                                       std::views::split('\n') |                    \
                                                       std::views::transform([](auto r) {           \
                                                         return std::string_view(r);                \
                                                       }),                                          \
                                                   "\n               "))));                         \
              __debugbreak();                                                                       \
          }                                                                                         \
      }                                                                                             \
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
