#pragma once

#include <chrono>
#include <concepts>
#include <locale>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <flecs.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/std.h>

namespace rl::io {
    const static std::locale locale{ "en_US.UTF-8" };

    enum class LogMode {
        LogFile = 1 << 0,
        StdOut = 1 << 1,
        StdErr = 1 << 2,
    };

    enum class LogLevel {
        Fatal,
        Error,
        Warning,
        Info,
        Debug,
        Trace
    };

    struct logging
    {
        constexpr static inline LogMode mode{ LogMode::StdOut };
        constexpr static inline LogLevel level{ LogLevel::Trace };

        constexpr static inline bool kb_events{ false };
        constexpr static inline bool mouse_events{ false };
        constexpr static inline bool window_events{ false };
        constexpr static inline bool main_loop{ true };
        constexpr static inline bool rendering{ false };
    };
}

namespace flecs {
    constexpr auto format_as(const flecs::string& str)
    {
        return fmt::string_view{ str.c_str() };
    }

    constexpr auto format_as(const flecs::string_view& str)
    {
        return fmt::string_view{ str.c_str() };
    }

    constexpr auto format_as(const flecs::entity& e)
    {
        return fmt::string_view{ e.name().c_str() };
    }
}

namespace rl::log {

    using namespace std::chrono_literals;

    template <auto log_level, typename... TArgs>
    constexpr inline void print(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        if constexpr (io::logging::level >= log_level)
        {
            fmt::text_style c{ fmt::fg(fmt::color(0xC1C4CA)) };
            switch (log_level)
            {
                case io::LogLevel::Trace:
                    c = fmt::fg(fmt::color(0xC1C4CA));
                    break;
                case io::LogLevel::Info:
                    c = fmt::fg(fmt::color(0x83B2B6));
                    break;
                case io::LogLevel::Debug:
                    c = fmt::fg(fmt::color(0x9AAF8B));
                    break;
                case io::LogLevel::Warning:
                    c = fmt::fg(fmt::color(0xCAB880));
                    break;
                case io::LogLevel::Error:
                    c = fmt::fg(fmt::color(0xD4A4A4));
                    break;
                case io::LogLevel::Fatal:
                    c = fmt::fg(fmt::color(0xB6ADDB));
                    break;
            }

            fmt::text_style style = c;
            fmt::print(style, fmt::format(format_str, std::forward<TArgs>(args)...) + "\n");
        }
    }

    template <typename... TArgs>
    constexpr inline void info(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log::print<io::LogLevel::Info>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    constexpr inline void debug(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log::print<io::LogLevel::Debug>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    constexpr inline void warning(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log::print<io::LogLevel::Warning>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    constexpr inline void error(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log::print<io::LogLevel::Error>(format_str, std::forward<TArgs>(args)...);
    }

    template <typename... TArgs>
    constexpr inline void fatal(fmt::format_string<TArgs...> format_str, TArgs&&... args)
    {
        log::print<io::LogLevel::Fatal>(format_str, std::forward<TArgs>(args)...);
        exit(-1);
    }
}
