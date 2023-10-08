#include "core/application.hpp"
#include "math/vector2d.hpp"
#include "utils/assert.hpp"

#include <argparse/argparse.hpp>
#include <iostream>

bool parse_args(int argc, char** argv)
{
    argparse::ArgumentParser args("roguelike", "0.0.1", argparse::default_arguments::none);

    // clang-format off
    args.add_argument("-h", "--help")
        .help("prints this message")
        .implicit_value(true);

    args.add_argument("-v", "--verbose")
        .help("enables verbose output")
        .implicit_value(true);

    args.add_argument("-c", "--console")
        .default_value("info")
        .help("specifies stdout message level");

    args.add_argument("-c", "--log")
        .default_value("error")
        .help("specifies logfile message level");

    // clang-format on

    try
    {
        args.parse_args(argc, argv);
        auto console_verbosity = args.get<std::string>("--console");
        return true;
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << args;
        runtime_assert(false, "Invalid program argument");
        return false;
    }
}

int main(int argc, char** argv)
{
    if (!parse_args(argc, argv))
        return -1;

    rl::dims2i screen_dims{ 800, 600 };
    std::string window_title{ "window title" };
    auto game = rl::Application(screen_dims, window_title);
    return game.run();
}
