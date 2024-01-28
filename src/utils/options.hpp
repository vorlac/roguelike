#pragma once

#include <array>

// #include <argparse/argparse.hpp>

#include "utils/io.hpp"

namespace rl {
    static bool parse_args(int argc, auto argv)
    {
        // auto args = argparse::ArgumentParser{
        //     "roguelike",
        //     "0.0.1",
        //     argparse::default_arguments::help,
        // };
        //
        // args.add_argument("-v", "--verbose")
        //     .help("enables verbose output")
        //     .default_value(true)
        //     .implicit_value(true);
        //
        // args.add_argument("-c", "--console")
        //     .default_value("info")
        //     .help("specifies stdout message level");
        //
        // args.add_argument("-c", "--log")
        //     .default_value("error")
        //     .help("specifies logfile message level");
        //
        // try
        // {
        //     args.parse_args(argc, argv);
        //     return true;
        // }
        // catch (const std::runtime_error& err)
        // {
        //     log::info("Failed to parse args: {}", err.what());
        //     return false;
        // }
        return true;
    }
}
