#pragma once

#include <type_traits>

#include "sdl/defs.hpp"
SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl {

    class System
    {
    public:
        struct Event
        {
            using type = SDL3::SDL_EventType;

            enum ID : std::underlying_type_t<type> {
                ClipboardUpdate = SDL3::SDL_EVENT_CLIPBOARD_UPDATE
            };
        };
    };
}
