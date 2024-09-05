#pragma once

#include <type_traits>

#include "utils/sdl_defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {
    class Display {
    public:
        struct Event {
            using type_t = SDL3::SDL_EventType;
            using type = std::underlying_type_t<type_t>;

            enum ID : type {
                // Display events
                // TODO: break out into object that manages display state
                DisplayFirst = SDL3::SDL_EVENT_DISPLAY_FIRST,
                DisplayOrientation = SDL3::SDL_EVENT_DISPLAY_ORIENTATION,
                DisplayAdded = SDL3::SDL_EVENT_DISPLAY_ADDED,
                DisplayRemoved = SDL3::SDL_EVENT_DISPLAY_REMOVED,
                DisplayMoved = SDL3::SDL_EVENT_DISPLAY_MOVED,
                DisplayContentScaleChanged = SDL3::SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED,
                DisplayLast = SDL3::SDL_EVENT_DISPLAY_LAST,
            };
        };
    };
}
