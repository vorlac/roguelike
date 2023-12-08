#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "core/numeric.hpp"
#include "sdl/defs.hpp"
#include "sdl/event_handler.hpp"
#include "sdl/time.hpp"
#include "sdl/window.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_init.h>
SDL_C_LIB_END

namespace rl::sdl {
    class Application
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
            All = Timer | Audio | Video | Joystick | Haptic | Gamepad | Events | Sensor
        };

    public:
        Application()
        {
            bool ret = this->init_subsystem(Subsystem::All);
            m_window = std::make_unique<sdl::Window>("Roguelite OpenGL");
        }

        Application(const sdl::Application& other) = delete;
        Application(sdl::Application&& other) = delete;

        ~Application()
        {
        }

        Application& operator=(const Application& other) = delete;
        Application& operator=(Application&& other) = delete;

        bool handle_events()
        {
            return m_event_handler.handle_events(m_window);
        }

        bool quit_triggered() const
        {
            return m_event_handler.quit_triggered();
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

        std::unique_ptr<sdl::Window>& window()
        {
            return m_window;
        }

    private:
        // TODO: implement single instance enforcement
        static inline std::atomic<bool> m_initialized{ false };
        std::once_flag init_flag{};

        std::unique_ptr<sdl::Window> m_window{};
        sdl::EventHandler m_event_handler{};
    };
}
