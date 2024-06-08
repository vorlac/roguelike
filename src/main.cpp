#include "core/application.hpp"
#include "utils/options.hpp"
#include "utils/signal/observer.hpp"

namespace rl::event {
    constexpr static auto signal1{ "test_signal_1" };
    constexpr static auto signal2{ "test_signal_2" };

}

int main(const int argc, char** argv)
{
    using namespace rl;
    auto callback_a = [] {
        fmt::println("Called: {}", "callback_a");
    };
    auto callback_b = [] {
        fmt::println("Called: {}", "callback_b");
    };

    Signal<event::signal1>::connect(callback_a);
    Signal<event::signal1>::connect(callback_b);
    Signal<event::signal1>::connect([] {
        fmt::println("Called: {}", "callback_c");
    });

    auto signal_one{ Signal<event::signal1>{} };
    signal_one.emit();

    Signal<event::signal1>::disconnect(callback_b);
    signal_one.emit();

    int ret{ -1 };
    if (rl::parse_args(argc, argv)) {
        rl::Application game{};
        ret = game.run();
    }

    return ret;
}
