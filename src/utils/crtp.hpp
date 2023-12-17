#pragma once

#include <memory>
#include <stack>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "core/state/states.hpp"

namespace rl {
    template <typename... TVisitorFunction>
    struct variant_visitor : TVisitorFunction...
    {
        template <typename... TCallable>
        variant_visitor(TCallable&&... vc)
            : TVisitorFunction{ std::forward<TCallable>(vc) }...
        {
        }

        using TVisitorFunction::operator()...;
    };

    template <typename... TVisitorFunction>
    variant_visitor(TVisitorFunction...)
        -> variant_visitor<std::remove_reference_t<TVisitorFunction>...>;

    template <typename... T>
    using static_polymophic_t = std::variant<T...>;

    using gamestate_vec = std::vector<      //
        static_polymophic_t<                //
            TeardownState, PauseMenuState,  //
            GameplayState, LoadLevelState,  //
            MainMenuState, GameInitState>>;

    using gamestate_stack = std::stack<     //
        static_polymophic_t<                //
            TeardownState, PauseMenuState,  //
            GameplayState, LoadLevelState,  //
            MainMenuState, GameInitState>,
        gamestate_vec>;
}

namespace rl::example {
    static void example()
    {
        gamestate_vec states = {
            GameInitState{},
            PauseMenuState{},
            GameplayState{},
            PauseMenuState{},
        };

        struct A
        {
            int a{};
        };

        struct B
        {
            int b{};
        };

        struct C
        {
            int c{};
        };

        struct D
        {
            int d{};
        };

        std::vector<std::variant<A, B, C, D>> variants = {
            A{ .a = 123 },
            B{ .b = 234 },
            D{ .d = 987 },
        };

        for (auto& var : variants)
        {
            std::visit(  //
                variant_visitor{
                    [](A& a_variant) {
                        std::cout << "A: " << a_variant.a << std::endl;
                    },
                    [](B& b_variant) {
                        std::cout << "B: " << b_variant.b << std::endl;
                    },
                    [](auto& other) {
                        if constexpr (std::same_as<decltype(other), C>)
                            std::cout << "C: " << other.c << std::endl;
                        if constexpr (std::same_as<decltype(other), D>)
                            std::cout << "D: " << other.d << std::endl;
                    },
                },
                var);
        }
    }
}
