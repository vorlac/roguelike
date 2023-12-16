#pragma once
#include <ranges>
#include <string>

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_error.h>
SDL_C_LIB_END

#ifdef NDEBUG
  #define runtime_assert(condition, fmtstr, ...) static_cast<void>(0)
  #define sdl_assert(condition, fmtstr, ...)     static_cast<void>(0)
  #define assert_cond(condition)                 static_cast<void>(0)
  #define assert_msg(message)                    static_cast<void>(0)

#else

  #define runtime_assert(condition, fmtstr, ...)                                                    \
      do                                                                                            \
      {                                                                                             \
          if (!(condition)) [[unlikely]]                                                            \
          {                                                                                         \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                             \
                         fmt::format("Assertion failed: ({}{}\n",                                   \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), #condition),        \
                                     fmt::format(fmt::fg(fmt::color(0xD4A4A4)), ")")));             \
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
                                                       std::views::transform([](auto r__) {         \
                                                         return std::string_view(r__);              \
                                                       }),                                          \
                                                   "\n               "))));                         \
              __debugbreak();                                                                       \
          }                                                                                         \
      }                                                                                             \
      while (0)

  #define sdl_assert(condition, fmtstr, ...)                                                       \
      do                                                                                           \
      {                                                                                            \
          if (!(condition)) [[unlikely]]                                                           \
          {                                                                                        \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                            \
                         fmt::format("Assertion failed: ({}{}\n",                                  \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), #condition),       \
                                     fmt::format(fmt::fg(fmt::color(0xD4A4A4)), ")")));            \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                            \
                         fmt::format("  Function = {}\n",                                          \
                                     fmt::format(fmt::fg(fmt::color(0xB6ADDB)), __FUNCTION__)));   \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                            \
                         fmt::format("  File     = {}\n",                                          \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), __FILE__)));       \
              fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                            \
                         fmt::format("  Line     = {}\n",                                          \
                                     fmt::format(fmt::fg(fmt::color(0xC1C4CA)), "{}", __LINE__))); \
              fmt::print(                                                                          \
                  fmt::fg(fmt::color(0xDCB4AA)),                                                   \
                  fmt::format("  Message  = {}",                                                   \
                              fmt::format(fmt::fg(fmt::color(0xCAB880)), "{}",                     \
                                          fmt::join(fmt::format(fmt::runtime(fmtstr)               \
                                                                    __VA_OPT__(, ) __VA_ARGS__) |  \
                                                        std::views::split('\n') |                  \
                                                        std::views::transform([](auto r__) {       \
                                                          return std::string_view(r__);            \
                                                        }),                                        \
                                                    "\n               "))));                       \
              fmt::print(                                                                          \
                  fmt::fg(fmt::color(0xDCB4AA)),                                                   \
                  fmt::format(" SDL Error = {}",                                                   \
                              fmt::format(fmt::fg(fmt::color(0xCAB880)), "{}",                     \
                                          fmt::join(fmt::format("{}", SDL3::SDL_GetError()) |      \
                                                        std::views::split('\n') |                  \
                                                        std::views::transform([](auto r__) {       \
                                                          return std::string_view(r__);            \
                                                        }),                                        \
                                                    "\n               ")))                         \
                      .c_str());                                                                   \
              __debugbreak();                                                                      \
          }                                                                                        \
      }                                                                                            \
      while (0)

  #define assert_msg(message) runtime_assert(false, message)
  #define assert_cond(cond)   runtime_assert(cond, "condition check failed")

#endif
