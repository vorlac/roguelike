#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <print>
#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rl::test::signals {
    namespace {
        template <size_t N>
        struct signal_name {
            const char name[N];
            constexpr static auto size = N;

            template <std::size_t... Is>
            constexpr signal_name(const char* s, std::index_sequence<Is...>)
                : name{ s[Is]... } {
            }

            constexpr signal_name(const char (&literal)[N])
                : signal_name(literal, std::make_index_sequence<N>{}) {
            }
        };

        template <std::size_t N>
        signal_name(const char (&)[N]) -> signal_name<N>;

        struct event_manager {
            using callback_t = std::function<void(std::string, std::string)>;
            static inline std::unordered_map<std::string, std::vector<callback_t>>
                Signals{};

            template <signal_name Event>
            constexpr static inline auto trigger_event(std::string message) {
                constexpr static std::string event_name{ Event.name, Event.size };
                Signals.try_emplace(event_name, std::vector<callback_t>{});
                auto& callbacks = Signals[event_name];
                for (auto& func : callbacks) {
                    func(event_name, message);
                }
            }

            template <signal_name Event>
            constexpr static inline auto add_signal() {
                constexpr static std::string signal = { Event.name, Event.size };
                Signals.try_emplace(signal, std::vector<callback_t>{});
                return [](std::string message) {
                    return [&] {
                        return trigger_event<Event>(message);
                    }();
                };
            };

            template <signal_name Event>
            struct signal_connection {
                constexpr inline void operator+=(callback_t& func) {
                    constexpr static std::string signal = { Event.name, Event.size };
                    Signals.try_emplace(signal, std::vector<callback_t>{});
                    Signals[signal].push_back(std::move(func));
                }
            };

            template <signal_name SignalName>
            static inline signal_connection<SignalName> connect{};
        };
    }

    void run_signal_test() {
        auto emit_asdf = event_manager::add_signal<"asdf">();

        event_manager::callback_t callback{
            [](std::string signal, std::string message) {
                return std::println("\nsignal: '{}' ==> invoked with message: '{}'",
                                    signal, message);
            }
        };

        event_manager::connect<"asdf"> += callback;

        emit_asdf("aaaaa");
    }
}
