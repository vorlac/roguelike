#pragma once

#ifdef __linux__
  #include <signal.h>
#endif

#include <ranges>

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/std.h>

#ifdef NDEBUG

  #define debug_assert(...) static_cast<void>(0)
  #define sdl_assert(...)   static_cast<void>(0)

#else

  #if defined(_WIN32)
    #define pause_debugger __debugbreak()
  #elif defined(__linux__)
    #define pause_debugger ::raise(SIGTRAP)
  #endif

  #define assert_dbg_(condition, fmtstr, ...)                                                \
      fmt::print(fmt::fg(fmt::color{ 0xDCB4AA }),                                            \
                 fmt::format("\nAssertion failed: ({}{}\n",                                  \
                             fmt::format(fmt::fg(fmt::color{ 0xC1C4CA }), #condition),       \
                             fmt::format(fmt::fg(fmt::color{ 0xD4A4A4 }), ")")));            \
      fmt::print(fmt::fg(fmt::color{ 0xDCB4AA }),                                            \
                 fmt::format("  Function = {}\n",                                            \
                             fmt::format(fmt::fg(fmt::color{ 0xB6ADDB }), __FUNCTION__)));   \
      fmt::print(fmt::fg(fmt::color{ 0xDCB4AA }),                                            \
                 fmt::format("  File     = {}\n",                                            \
                             fmt::format(fmt::fg(fmt::color{ 0xC1C4CA }), __FILE__)));       \
      fmt::print(fmt::fg(fmt::color{ 0xDCB4AA }),                                            \
                 fmt::format("  Line     = {}\n",                                            \
                             fmt::format(fmt::fg(fmt::color{ 0xC1C4CA }), "{}", __LINE__))); \
      if (std::string{ #fmtstr }.size() > 0) {                                               \
          fmt::print(fmt::fg(fmt::color{ 0xDCB4AA }),                                        \
                     fmt::format(                                                            \
                         "  Message  = {}\n",                                                \
                         fmt::format(fmt::fg(fmt::color{ 0xCAB880 }), "{}",                  \
                                     fmt::join(fmt::format(fmt::runtime(#fmtstr)             \
                                                               __VA_OPT__(, ) __VA_ARGS__) | \
                                                   std::views::split('\n') |                 \
                                                   std::views::transform([](auto r__) {      \
                                                     return std::string_view(r__);           \
                                                   }),                                       \
                                               "\n             "))));                        \
      }

  #define assert_sdl_(condition, fmtstr, ...)                                                \
      fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                              \
                 fmt::format("\nAssertion failed: ({}{}\n",                                  \
                             fmt::format(fmt::fg(fmt::color(0xC1C4CA)), #condition),         \
                             fmt::format(fmt::fg(fmt::color(0xD4A4A4)), ")")));              \
      fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                              \
                 fmt::format("  Function = {}\n",                                            \
                             fmt::format(fmt::fg(fmt::color(0xB6ADDB)), __FUNCTION__)));     \
      fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                              \
                 fmt::format("  File     = {}\n",                                            \
                             fmt::format(fmt::fg(fmt::color(0xC1C4CA)), __FILE__)));         \
      fmt::print(fmt::fg(fmt::color(0xDCB4AA)),                                              \
                 fmt::format("  Line     = {}\n",                                            \
                             fmt::format(fmt::fg(fmt::color(0xC1C4CA)), "{}", __LINE__)));   \
      fmt::print(                                                                            \
          fmt::fg(fmt::color(0xDCB4AA)),                                                     \
          fmt::format("  Message  = {}\n",                                                   \
                      fmt::format(fmt::fg(fmt::color(0xCAB880)), "{}",                       \
                                  fmt::join(fmt::format(fmtstr __VA_OPT__(, ) __VA_ARGS__) | \
                                                std::views::split('\n') |                    \
                                                std::views::transform([](auto r_) {          \
                                                  return std::string_view(r_);               \
                                                }),                                          \
                                            "\n               "))));                         \
      fmt::print(                                                                            \
          fmt::fg(fmt::color{ 0xDCB4AA }),                                                   \
          fmt::format(" SDL Error = {}\n",                                                   \
                      fmt::format(fmt::fg(fmt::color{ 0xCAB880 }), "{}",                     \
                                  fmt::join(fmt::format("{}", SDL3::SDL_GetError()) |        \
                                                std::views::split('\n') |                    \
                                                std::views::transform([](auto r_) {          \
                                                  return std::string_view(r_);               \
                                                }),                                          \
                                            "\n               "))))

  #define assert_msg_(fmtstr, ...) \
      assert_dbg_(fmtstr __VA_OPT__(, ) __VA_ARGS__, fmtstr __VA_OPT__(, ) __VA_ARGS__)

  #define assert_cond_(cond) \
      assert_dbg_(cond, )

  #define debug_assert(cond, ...)                                                           \
      do {                                                                                  \
          using cond_t = std::decay_t<decltype(cond)>;                                      \
          if constexpr (std::same_as<cond_t, const char*>) {                                \
              assert_msg_(cond __VA_OPT__(, ) __VA_ARGS__);                                 \
              pause_debugger;                                                               \
          }                                                                                 \
          else if (!(cond)) [[unlikely]] { /* NOLINT(clang-diagnostic-string-conversion) */ \
              const std::tuple args{ std::make_tuple(__VA_ARGS__) };                        \
              constexpr uint32_t arg_count{ std::tuple_size_v<decltype(args)> };            \
              if constexpr (arg_count > 0) {                                                \
                  __VA_OPT__(assert_dbg_(cond, __VA_ARGS__));                               \
              }                                                                             \
              else if constexpr (arg_count == 0) {                                          \
                  assert_cond_(cond);                                                       \
              }                                                                             \
              pause_debugger;                                                               \
          }                                                                                 \
      }                                                                                     \
      while (false)

  #define sdl_assert(cond, ...)                             \
      do {                                                  \
          if (!(cond)) [[unlikely]] {                       \
              assert_sdl_(cond __VA_OPT__(, ) __VA_ARGS__); \
              pause_debugger;                               \
          }                                                 \
      }                                                     \
      while (false)

#endif
