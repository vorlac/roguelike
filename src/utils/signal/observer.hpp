#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/format.h>

namespace rl::inline utils {

    template <typename... T>
    struct test
    {
        test(T&&... tup)
            : m_objects{ std::forward<T>(tup)... }
        {
        }

        template <std::invocable TCallable>
        void apply(TCallable&& callable)
        {
            std::apply(
                [&](auto&&... args) {
                    (std::invoke(std::forward<TCallable>(callable(std::forward<decltype(args)>(args)))), ...);
                },
                m_objects);
        }

    private:
        std::tuple<T...> m_objects{};
    };

    constexpr std::string_view signal_a{ "signal_A" };
    constexpr std::string_view signal_b{ "signal_B" };

    class Observer
    {
    public:
        virtual ~Observer() {};
        virtual void notify() const = 0;
    };

    template <auto& SignalName>
        requires std::same_as<std::string_view, std::remove_cvref_t<decltype(SignalName)>>
    class Signal
    {
    public:
        constexpr static std::string_view name{ SignalName };

        void connect(Observer* observer)
        {
            m_observers.push_back(observer);
        }

        void disconnect(Observer* observer)
        {
            fmt::println("Disconnect");
            m_observers.remove(observer);
        }

        void emit() const
        {
            for (const auto& observer : m_observers)
                observer->notify();
        }

    private:
        std::list<Observer*> m_observers{};
    };

    template <auto& Name>
    class ObserverA : public Observer
    {
    public:
        ObserverA(Signal<Name>& subject)
            : m_signal{ subject }
        {
            m_signal.get().connect(this);
        }

        virtual void notify() const override
        {
            fmt::print("Observer A notified from {}\n", Name);
        }

    private:
        std::reference_wrapper<Signal<Name>> m_signal;
    };

    template <auto& Name>
    class ObserverB : public Observer
    {
    public:
        ObserverB(Signal<Name>& subject)
            : m_signal{ subject }
        {
            m_signal.get().connect(this);
        }

        virtual void notify() const override
        {
            fmt::print("Observer B notified from {}\n", Name);
        }

    private:
        std::reference_wrapper<Signal<Name>> m_signal;
    };

    template <auto Name>
    ObserverA(Signal<Name>) -> ObserverA<Name>;
    template <auto Name>
    ObserverB(Signal<Name>) -> ObserverB<Name>;

    int main()
    {
        rl::test t{ 1, "2", 3.14 };
        t.apply(std::function<void(auto&&)>([](auto&& obj) {
            fmt::print("{}\n", std::forward<decltype(obj)>(obj));
        }));

        fmt::print("\n");

        Signal<signal_a> sig_a{};
        Signal<signal_b> sig_b{};
        ObserverA observer_a{ sig_a };
        ObserverB observer_b{ sig_a };
        sig_b.connect(&observer_a);
        sig_b.connect(&observer_b);

        sig_b.emit();
        sig_a.emit();

        sig_a.disconnect(&observer_a);
        sig_a.emit();

        fmt::print("\n");
    }

}
