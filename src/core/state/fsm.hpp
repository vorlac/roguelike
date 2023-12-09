#pragma once

#include <any>
#include <memory>
#include <stack>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <variant>

#include "core/state/states.hpp"
#include "utils/assert.hpp"
#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

namespace rl {
    class StateMachine
    {
    public:
        template <typename T>
        void push(T&& s)
        {
            // m_states.emplace(std::forward<T>(s));
            // auto&& state = m_states.top();
            // using test_type = std::type_identity<T>;
            // using state_type = std::type_identity_t<decltype(state)>::value_type;
            // std::any_cast<test_type>(state).on_enter();
            // m_states.top()->on_enter();
        }

        void pop(auto&& s)
        {
            // runtime_assert(m_states.size() > 0, "FSM has no active state");
            // m_states.top().on_exit();
            // m_states.pop();
        }

        auto current()
        {
            // runtime_assert(m_states.size() > 0, "FSM has no active state");
            // auto& ret = m_states.top();
            // return ret;
            // static_cast<typeid(std::decay_t<decltype(ret)>(ret).type().hash_code)>(ret);
            // return static_cast<typeid(rettype())>(ret);
        }

        [[nodiscard]]
        u32 size() noexcept
        {
            return 0;  // return static_cast<u32>(m_states.size());
        }

        // private:
        //  GameStateView m_states = { GameInitState() };
        //   std::stack<std::variant<auto>> m_states{};
    };
}
