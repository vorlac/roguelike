#include <chrono>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/game.hpp"
#include "core/numeric_types.hpp"

namespace SDL3
{
#include <SDL3/SDL_events.h>
}

namespace rl
{
    bool Game::setup()
    {
        return m_sdl.is_initialized();
    }

    static bool quit_requested()
    {
        SDL3::SDL_PumpEvents();
        return SDL3::SDL_PeepEvents(nullptr, 0, SDL3::SDL_PEEKEVENT, SDL3::SDL_EVENT_QUIT,
                                    SDL3::SDL_EVENT_QUIT) > 0;
    }

    bool Game::run()
    {
        this->setup();

        u64 i = 0;
        while (!quit_requested())
        {
            ++i;
            fmt::print("{}\n", i);
            using namespace std::literals;
            std::this_thread::sleep_for(0.1s);
        }

        this->teardown();
        return true;
    }

    bool Game::teardown()
    {
        return true;
    }

    bool Game::print_log_msg(const char* event_type, const char* text)
    {
        char expanded[1024] = { 0 };
        for (const char* spot = text; *spot; ++spot)
        {
            size_t length = SDL3::SDL_strlen(expanded);
            SDL3::SDL_snprintf(expanded + length, sizeof(expanded) - length, "\\x%.2x", (u8)*spot);
        }
        SDL3::SDL_Log("%s Text (%s): \"%s%s\"\n", event_type, expanded, *text == '"' ? "\\" : "",
                      text);
        return true;
    }

    bool Game::process_inputs() const
    {
        static constexpr i32 KeyPressed  = 1;
        static constexpr i32 KeyReleased = 0;

        static constexpr i32 MouseButtonLeft   = 1;
        static constexpr i32 MouseButtonMiddle = 2;
        static constexpr i32 MouseButtonRight  = 3;

        SDL3::SDL_Event input_event;
        while (SDL3::SDL_PollEvent(&input_event))
        {
            switch (static_cast<SDL3::SDL_EventType>(input_event.type))
            {
                case SDL3::SDL_EVENT_KEY_DOWN:
                case SDL3::SDL_EVENT_KEY_UP:
                    // auto& keysym = input_event.key.keysym;
                    //(input_event.key.state == KeyPressed) ? SDL3::SDL_TRUE : SDL3::SDL_FALSE;
                    //(input_event.key.repeat) ? SDL3::SDL_TRUE : SDL3::SDL_FALSE;

                    break;
                case SDL3::SDL_EVENT_TEXT_EDITING:
                    break;
                case SDL3::SDL_EVENT_TEXT_EDITING_EXT:
                    break;
                case SDL3::SDL_EVENT_TEXT_INPUT:
                    break;
                case SDL3::SDL_EVENT_FINGER_DOWN:
                    if (SDL3::SDL_TextInputActive())
                    {
                        SDL3::SDL_Log("Stopping text input\n");
                        SDL3::SDL_StopTextInput();
                    }
                    else
                    {
                        SDL3::SDL_Log("Starting text input\n");
                        SDL3::SDL_StartTextInput();
                    }
                    break;
                case SDL3::SDL_EVENT_MOUSE_BUTTON_DOWN:
                    /* Left button quits the app, other buttons toggles text input */
                    if (input_event.button.button == MouseButtonLeft)
                    {
                        //
                    }
                    else if (SDL3::SDL_TextInputActive())
                    {
                        SDL3::SDL_Log("Stopping text input\n");
                        SDL3::SDL_StopTextInput();
                    }
                    else
                    {
                        SDL3::SDL_Log("Starting text input\n");
                        SDL3::SDL_StartTextInput();
                    }
                    break;
                case SDL3::SDL_EVENT_QUIT:
                    SDL3::SDL_Quit();
                    return true;
                default:
                    break;
            }
        }

        return false;
    }

    void Game::quit()
    {
    }
}
