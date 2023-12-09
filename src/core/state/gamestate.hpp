#pragma once

#include <memory>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/window.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl {
    class StateMachine;

    class GameState
    {
    public:
        void on_enter(this auto&& self)
        {
            self.enter();
        }

        void handle_events(this auto&& self, auto&& e, StateMachine& fsm)
        {
            self.process_event(e, fsm);
        }

        void update_components(this auto&& self)
        {
            self.update();
        }

        void on_exit(this auto&& self)
        {
            self.exit();
        }

        // private:
        //     void enter()
        //     {
        //         runtime_assert(false, "enter()");
        //     }

        //    void process_event(auto&&, StateMachine& fsm)
        //    {
        //        runtime_assert(false, "process_event()");
        //    }

        //    void update()
        //    {
        //        runtime_assert(false, "update()");
        //    }

        //    void exit()
        //    {
        //        runtime_assert(false, "exit()");
        //    }
    };

}
