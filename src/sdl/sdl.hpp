#pragma once

#include <atomic>
#include <string>

#include "core/numeric_types.hpp"
#include "sdl/renderer.hpp"
#include "sdl/window.hpp"

namespace SDL3
{
#include <SDL3/SDL.h>
}

namespace rl::sdl
{
    class sdl_app
    {
    public:
        enum Subsystem : u16_fast {
            Timer    = SDL3::SDL_INIT_TIMER,
            Audio    = SDL3::SDL_INIT_AUDIO,
            Video    = SDL3::SDL_INIT_VIDEO,
            Joystick = SDL3::SDL_INIT_JOYSTICK,
            Haptic   = SDL3::SDL_INIT_HAPTIC,
            Gamepad  = SDL3::SDL_INIT_GAMEPAD,
            Events   = SDL3::SDL_INIT_EVENTS,
            Sensor   = SDL3::SDL_INIT_SENSOR,

            Count = Sensor,

            All = SDL3::SDL_INIT_TIMER |     //
                  SDL3::SDL_INIT_AUDIO |     //
                  SDL3::SDL_INIT_VIDEO |     //
                  SDL3::SDL_INIT_JOYSTICK |  //
                  SDL3::SDL_INIT_HAPTIC |    //
                  SDL3::SDL_INIT_GAMEPAD |   //
                  SDL3::SDL_INIT_EVENTS |    //
                  SDL3::SDL_INIT_SENSOR
        };

    public:
        sdl_app()
            : sdl_app(Subsystem::All)
        {
        }

        sdl_app(Subsystem flags)
        {
            this->init_subsystem(flags);
        }

        ~sdl_app()
        {
            SDL3::SDL_Quit();
        }

        bool is_initialized() const
        {
            return m_initialized.load(std::memory_order_relaxed);
        }

        bool init_subsystem(Subsystem flags)
        {
            bool init{ m_initialized.load(std::memory_order_relaxed) };
            m_initialized.store(!init && 0 == SDL3::SDL_Init(flags), std::memory_order_relaxed);
            return m_initialized.load(std::memory_order_relaxed);
        }

    private:
        sdl_app(const sdl::sdl_app& other)       = delete;
        sdl_app& operator=(const sdl_app& other) = delete;
        sdl_app(sdl::sdl_app&& other)            = delete;
        sdl_app& operator=(sdl_app&& other)      = delete;

        void report_error()
        {
            // SDL3::SDL_GetErrBuf();
        }

    private:
        // guard that makes sure only one exists
        static inline std::atomic<bool> m_initialized{ false };
        sdl::window m_window{};
        sdl::renderer m_renderer{};
    };
}
