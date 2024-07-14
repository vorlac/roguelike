#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <fmt/format.h>
#include <parallel_hashmap/phmap.h>

#include "utils/concepts.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"
#include "utils/reflect.hpp"

namespace rl::inline utils {

    template <auto& SignalName, typename TSubscriber = std::nullopt_t, std::invocable TCallable = std::function<void()>>
        requires rl::any_of<std::remove_cvref_t<decltype(SignalName)>, std::string_view, const char*, const char[]>
    class Signal
    {
    public:
        using callable_t = TCallable;
        using subscriber_t = std::remove_cvref_t<TSubscriber>;
        constexpr static std::string_view name{ SignalName };
        constexpr static std::string_view hash_name{ reflect::demangled_typename<callable_t>() };

        Signal() = default;

        static void connect(TSubscriber&&, TCallable&& slot)
            requires(!std::same_as<std::nullopt_t, subscriber_t>)
        {
            m_subscribers.emplace(hash_name, std::forward<TCallable>(slot));
        }

        static void connect(TCallable&& slot)
            requires(std::same_as<std::nullopt_t, subscriber_t>)
        {
            m_subscribers.emplace(hash_name, std::forward<TCallable>(slot));
        }

        static void disconnect(TCallable&&)
        {
            m_subscribers.erase(hash_name);
        }

        void emit() noexcept
        {
            for (auto&& [key, slot] : m_subscribers)
                std::invoke(std::forward<decltype(slot)>(slot));
        }

        bool operator==(const Signal& other) const
        {
            debug_assert(hash_value(*this) == hash_value(other));
            return hash_value(*this) == hash_value(other);
        }

        friend std::size_t hash_value(const TSubscriber& subscriber)
        {
            debug_assert(hash_name == Signal::hash_name);
            debug_assert(hash_name == decltype(subscriber)::hash_name);
            return std::hash(Signal::hash_name);
        }

    protected:
        Signal(const TSubscriber, TCallable&&) = delete;
        Signal(TCallable&&) = delete;

    private:
        static inline phmap::flat_hash_map<std::string_view, TCallable> m_subscribers{};
    };

    // template <auto& SignalName, typename TSubscriber, std::invocable TCallable>
    // Signal(TSubscriber, TCallable) -> Signal<SignalName, TSubscriber, TCallable>;
    // template <auto& SignalName, typename TSubscriber, std::invocable TCallable>
    // Signal(TCallable) -> Signal<SignalName, void, TCallable>;
    // template <auto& SignalName, typename TSubscriber, std::invocable TCallable>
    // Signal() -> Signal<SignalName, std::void_t<void>>;
}

// namespace std {
//     // inject specialization of std::hash for Person into namespace std
//     // ----------------------------------------------------------------
//     template <>
//     struct hash<Person>
//     {
//         std::size_t operator()(const Person& p) const
//         {
//             return phmap::HashState().combine(0, p._first, p._last, p._age);
//         }
//     };
// }

// class Observer
//{
// public:
//     virtual ~Observer() {};
//     virtual void notify() const = 0;
// };

// template <auto& Name>
// class ObserverA : public Observer
//{
// public:
//     ObserverA(Signal<Name>& subject)
//         : m_signal{ subject }
//     {
//         m_signal.get().connect(this);
//     }

//    virtual void notify() const override
//    {
//        fmt::print("Observer A notified from {}\n", Name);
//    }

// private:
//     std::reference_wrapper<Signal<Name>> m_signal;
// };

// template <auto& Name>
// class ObserverB : public Observer
//{
// public:
//     ObserverB(Signal<Name>& subject)
//         : m_signal{ subject }
//     {
//         m_signal.get().connect(this);
//     }

//    virtual void notify() const override
//    {
//        fmt::print("Observer B notified from {}\n", Name);
//    }

// private:
//     std::reference_wrapper<Signal<Name>> m_signal;
// };

// template <auto Name>
// ObserverA(Signal<Name>) -> ObserverA<Name>;
// template <auto Name>
// ObserverB(Signal<Name>) -> ObserverB<Name>;

// template <typename... T>
// struct test
//{
//     test(T&&... tup)
//         : m_objects{ std::forward<T>(tup)... }
//     {
//     }

//    template <std::invocable TCallable>
//    void apply(TCallable&& callable)
//    {
//        std::apply(
//            [&](auto&&... args) {
//                (std::invoke(std::forward<TCallable>(callable(std::forward<decltype(args)>(args)))), ...);
//            },
//            m_objects);
//    }

// private:
//     std::tuple<T...> m_objects{};
// };

// constexpr std::string_view signal_a{ "signal_A" };
// constexpr std::string_view signal_b{ "signal_B" };

// int test_signals()
//{
//     rl::test t{ 1, "2", 3.14 };
//     t.apply(std::function<void(auto&&)>([](auto&& obj) {
//         fmt::print("{}\n", std::forward<decltype(obj)>(obj));
//     }));

//    fmt::print("\n");

//    Signal<signal_a> sig_a{};
//    Signal<signal_b> sig_b{};
//    ObserverA observer_a{ sig_a };
//    ObserverB observer_b{ sig_a };
//    sig_b.connect(&observer_a);
//    sig_b.connect(&observer_b);

//    sig_b.emit();
//    sig_a.emit();

//    sig_a.disconnect(&observer_a);
//    sig_a.emit();

//    fmt::print("\n");
//}
