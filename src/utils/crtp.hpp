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
        // this ctor is needed so that the forward call allows it to handle
        // callable types that are move-only or something like a std::function
        // that's already been created or a function pointer, etc...
        template <typename... TCallable>
        variant_visitor(TCallable&&... vc)
            : TVisitorFunction{ std::forward<TCallable>(vc) }...
        {
        }

        // define this struct so it uses the
        // operator() of every template arg type.
        // this way
        using TVisitorFunction::operator()...;
    };

    //  type deduction for variant_visitor
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

        for (auto& state : states)
        {
            std::visit(  //
                variant_visitor{
                    [](PauseMenuState& pause_state) {
                        int a = 0;
                        return a;
                    },
                    [](GameplayState& gameplay_state) {
                        int a = 0;
                        return a;
                    },
                    [](auto& other_state) {
                        int a = 0;
                        return a;
                    },
                },
                state);
        }
    }
}
