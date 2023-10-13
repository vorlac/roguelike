#include <string>
#include <argparse/argparse.hpp>

#include "core/game.hpp"
#include "ds/vector2d.hpp"
#include "utils/assert.hpp"

bool parse_args(int argc, char** argv)
{
    auto args = argparse::ArgumentParser{ "roguelike", "0.0.1", argparse::default_arguments::help };

    try
    {
        // clang-format off
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
        args.parse_args(argc, argv);

        // get arg example
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

    rl::Game game{};
    return game.run();
}
