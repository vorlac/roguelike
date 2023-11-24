#pragma once

#include <any>
#include <array>
#include <optional>

#include <argparse/argparse.hpp>

#include "utils/io.hpp"

namespace rl {
    struct options
    {
        struct log
        {
            enum class Mode {
                LogFile = 1 << 0,
                StdOut = 1 << 1,
                StdErr = 1 << 2,
            };

            enum class Level {
                Trace,
                Debug,
                Info,
                Warning,
                Error,
                Fatal
            };

            constexpr static inline Mode mode{ Mode::StdOut };
            constexpr static inline Level level{ Level::Debug };

            constexpr static inline bool kb_events{ false };
            constexpr static inline bool mouse_events{ false };
            constexpr static inline bool window_events{ true };
            constexpr static inline bool main_loop{ false };
            constexpr static inline bool rendering{ false };
        };
    };

    static bool parse_args(int argc, char** argv)
    {
        auto args = argparse::ArgumentParser{
            "roguelike",
            "0.0.1",
            argparse::default_arguments::help,
        };

        args.add_argument("-v", "--verbose")
            .help("enables verbose output")
            .default_value(true)
            .implicit_value(true);

        args.add_argument("-c", "--console")
            .default_value("info")
            .help("specifies stdout message level");

        args.add_argument("-c", "--log")
            .default_value("error")
            .help("specifies logfile message level");

        try
        {
            args.parse_args(argc, argv);
            return true;
        }
        catch (const std::runtime_error& err)
        {
            log::info("Failed to parse args: {}", err.what());
            return false;
        }
    }
}
