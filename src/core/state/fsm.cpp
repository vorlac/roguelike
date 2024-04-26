#include <memory>
#include <stack>
#include <type_traits>

#include "core/state/fsm.hpp"

namespace rl {

    // template <typename T>
    // void StateMachine::push(state_t&& s)
    //{
    //     m_states.push(std::move(s));
    //     m_states.top()->on_enter();
    // }

    // void StateMachine::pop(state_t&& s)
    //{
    //     debug_assert(m_states.size() > 0, "FSM has no active state");
    //     m_states.top()->on_exit();
    //     m_states.pop();
    // }

    // state_t& StateMachine::current()
    //{
    //     debug_assert(m_states.size() > 0, "FSM has no active state");
    //     return m_states.top();
    // }
}
