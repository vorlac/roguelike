#pragma once

#include <chrono>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <locale>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/std.h>

namespace rl::io {
    static const std::locale locale{ "en_US.UTF-8" };

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
        constexpr static inline bool mouse_move_events{ false };
        constexpr static inline bool window_events{ true };
        constexpr static inline bool gui_events{ true };
        constexpr static inline bool main_loop{ false };
        constexpr static inline bool rendering{ false };
    };
}

namespace rl::io {

    using namespace std::chrono_literals;

    struct ScopedWriter
    {
        explicit ScopedWriter(std::filesystem::path file_path, int32_t flags = std::fstream::binary | std::fstream::app)
            : m_write_stream{
                std::move(file_path),
                std::fstream::out | flags,
            }
        {
            debug_assert(m_write_stream.is_open());
        }

        ~ScopedWriter()
        {
            if (m_write_stream.is_open())
                m_write_stream.close();
        }

        template <typename T>
            requires std::is_trivially_copyable_v<T>
        std::size_t write(const auto&& data)
        {
            // binary write example...
            debug_assert(m_write_stream.is_open());
#pragma pack(4)  // force 4 byte alignment to avoid most unwanted padding
            m_write_stream.write(static_cast<uint8_t*>(&data), sizeof(data));
            m_write_stream.flush();
#pragma pack()
        }

        std::size_t write(const std::string& data)
        {
            // make sure to use text mode instead of binary mode if you use something like this
            debug_assert(m_write_stream.is_open());
            m_write_stream << data;  // write data to file stream
            m_write_stream.flush();  // flush data to disk
            m_write_stream.sync();   // sync with phyiscal IO device
            debug_assert(!m_write_stream.fail(), "failed to write: {}", data);
            debug_assert(!m_write_stream.bad(), "encountered bad bit when writing: {}", data);
            return data.size();
        }

    private:
        std::fstream m_write_stream{};
    };
}
