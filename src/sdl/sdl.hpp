#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "core/numeric_types.hpp"
#include "sdl/events.hpp"
#include "sdl/renderer.hpp"
#include "sdl/time.hpp"
#include "sdl/window.hpp"

namespace SDL3 {
#include <SDL3/SDL_blendmode.h>
}

namespace rl::sdl {

    class application
    {
    public:
        enum Subsystem : u16_fast {
            Timer = SDL3::SDL_INIT_TIMER,
            Audio = SDL3::SDL_INIT_AUDIO,
            Video = SDL3::SDL_INIT_VIDEO,
            Joystick = SDL3::SDL_INIT_JOYSTICK,
            Haptic = SDL3::SDL_INIT_HAPTIC,
            Gamepad = SDL3::SDL_INIT_GAMEPAD,
            Events = SDL3::SDL_INIT_EVENTS,
            Sensor = SDL3::SDL_INIT_SENSOR,

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

        using timer_t = sdl::perftimer<double, sdl::TimeDuration::Millisecond>;

    public:
        application()
        {
            bool ret = this->init_subsystem(Subsystem::Video);
            runtime_assert(ret, "failed to init SDL subsystem");
        }

        ~application()
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
            i32 result = SDL3::SDL_Init(flags);
            runtime_assert(result == 0, "failed to init subsystem");
            m_initialized.store(!init && 0 == SDL3::SDL_Init(flags), std::memory_order_relaxed);
            return m_initialized.load(std::memory_order_relaxed);
        }

        sdl::window& window()
        {
            return m_window;
        }

    private:
        application(const sdl::application& other) = delete;
        application(sdl::application&& other) = delete;

        application& operator=(const application& other) = delete;
        application& operator=(application&& other) = delete;

    private:
        // TODO: implement single instance enforcement
        static inline std::atomic<bool> m_initialized{ false };
        std::once_flag init_flag{};

        sdl::window m_window = { "Roguelite" };

        sdl::texture m_sprite{ m_window.renderer(), SDL3::SDL_PIXELFORMAT_ARGB8888,
                               SDL3::SDL_TEXTUREACCESS_STATIC, 4, 4 };
        sdl::texture m_target1{ m_window.renderer(), SDL3::SDL_PIXELFORMAT_ARGB8888,
                                SDL3::SDL_TEXTUREACCESS_TARGET, 512, 512 };
        sdl::texture m_target2{ m_window.renderer(), SDL3::SDL_PIXELFORMAT_ARGB8888,
                                SDL3::SDL_TEXTUREACCESS_TARGET, 512, 512 };
    };
}
