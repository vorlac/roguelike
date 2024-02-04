#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

#include <fmt/color.h>
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/common.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "utils/fs.hpp"
#include "utils/numeric.hpp"

#ifdef NDEBUG

  #define scoped_log(...)         static_cast<void>(0)
  #define scoped_logger(dbg, ...) static_cast<void>(0)
  #define scoped_log(...)         static_cast<void>(0)
  #define diag_log(...)           static_cast<void>(0)

#else

  #define scoped_trace(dbg)                                                    \
      std::string _f{ fmt::to_string(                                          \
          fmt::format("{}", this->name() + "::" + std::string{ __func__ })) }; \
      rl::ScopedLogger _lg(std::move(_f), dbg)

  #define scoped_logger(dbg, ...)                                                                  \
      std::string _f{ fmt::to_string(                                                              \
          fmt::format("{}", this->name() + "::" + std::string{ __func__ })) };                     \
      __VA_OPT__(std::string _o{ fmt::format(" => {}", fmt::format(__VA_ARGS__)) }; _f = _f + _o;) \
      rl::ScopedLogger _lg(std::move(_f), dbg)

  #define scoped_log(...)                                                                          \
      std::string _f{ fmt::to_string(                                                              \
          fmt::format("{}", this->name() + "::" + std::string{ __func__ })) };                     \
      __VA_OPT__(std::string _o{ fmt::format(" => {}", fmt::format(__VA_ARGS__)) }; _f = _f + _o;) \
      rl::ScopedLogger _lg(std::move(_f))

  #define diag_log(...)                              \
      std::string _diag{ fmt::format(__VA_ARGS__) }; \
      _lg.inner_scope_diag(std::move(_diag))

#endif

namespace rl {
    using log_level = spdlog::level::level_enum;

    class ScopedLogger
    {
        constexpr static inline log_level LOGFILE_LEVEL{ log_level::info };
        constexpr static inline log_level STD_OUT_LEVEL{ log_level::warn };

    public:
        explicit inline ScopedLogger(std::string&& str)
            : m_log_str{ std::forward<decltype(str)>(str) }
            , m_level{ log_level::info }
        {
            m_logger->log(m_level, "{:{}}-> {}", "", ++m_depth * INDENT, m_log_str);
        }

        explicit inline ScopedLogger(std::string&& str, log_level log_level)
            : m_log_str{ std::forward<decltype(str)>(str) }
            , m_level{ log_level }
        {
            m_logger->log(m_level, "{:{}}-> {}", "", ++m_depth * INDENT, m_log_str);
        }

        explicit inline ScopedLogger(log_level log_level)
            : m_level{ log_level }
        {
            m_logger->log(m_level, "{:{}}-> {}", "", ++m_depth * INDENT, m_log_str);
        }

        inline ~ScopedLogger()
        {
            m_logger->log(m_level, "{:{}}<- {}", "", m_depth-- * INDENT, m_log_str);
        }

        void inner_scope_diag(std::string&& str)
        {
            m_logger->log(m_level, "{:{}}   {}", "", m_depth * INDENT,
                          std::forward<std::string>(str));
        }

    private:
        static inline bool init_sinks()
        {
            constexpr log_level detail_level{ std::min(LOGFILE_LEVEL, STD_OUT_LEVEL) };
            auto stdout_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>(
                spdlog::color_mode::always);
            auto logfile_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                fs::absolute("../../../log/roguelike_trace.log").string(),
                ScopedLogger::MAX_LOGFILE_SIZE, ScopedLogger::MAX_LOGFILE_COUNT, true);

            stdout_sink->set_pattern("[%^%L%$] %v");
            stdout_sink->set_level(STD_OUT_LEVEL);

            logfile_sink->set_pattern("%^[%I:%M:%S.%e %p] [%L]%$ %v");
            logfile_sink->set_level(LOGFILE_LEVEL);

            spdlog::set_default_logger(m_logger);
            m_logger = std::shared_ptr<spdlog::logger>{
                new spdlog::logger{
                    "scoped_loggers",
                    { stdout_sink, logfile_sink },
                },
            };

            m_logger->set_level(detail_level);
            spdlog::set_level(detail_level);

            return true;
        }

    private:
        ScopedLogger(ScopedLogger&& logger) = delete;
        ScopedLogger(const ScopedLogger& logger) = delete;
        ScopedLogger& operator=(ScopedLogger&& logger) = delete;
        ScopedLogger& operator=(const ScopedLogger& logger) = delete;

    private:
        constexpr static inline i32 BYTES_PER_KB{ 1024 };
        constexpr static inline i32 BYTES_PER_MB{ 1024 * BYTES_PER_KB };
        constexpr static inline i32 MAX_LOGFILE_SIZE{ 15 * BYTES_PER_MB };
        constexpr static inline i32 MAX_LOGFILE_COUNT{ 1 };
        constexpr static inline i32 INDENT{ 3 };

    private:
        static inline std::shared_ptr<spdlog::logger> m_logger{};
        static inline bool m_initialized{ ScopedLogger::init_sinks() };
        static inline thread_local u32 m_depth{ 0 };

        log_level m_level{ log_level::info };
        std::string m_log_str{};
        bool m_print{ false };
    };
}
