#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include <glad/gl.h>

#include "core/numeric_types.hpp"
#include "sdl/defs.hpp"
#include "sdl/events.hpp"
#include "sdl/renderer.hpp"
#include "sdl/time.hpp"
#include "sdl/window.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_video.h>
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

        // using timer_t = sdl::Timer<double, sdl::TimeDuration::Millisecond>;

    public:
        Application()
        {
            bool ret = this->init_subsystem(Subsystem::Video);

            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_DOUBLEBUFFER, 1);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MINOR_VERSION, 2);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_PROFILE_MASK,
                                      SDL3::SDL_GL_CONTEXT_PROFILE_CORE);

            m_window = sdl::Window{ "SDL3 OpenGL" };
            runtime_assert(ret, "failed to init SDL subsystem");

            SDL3::SDL_GLContext context = SDL3::SDL_GL_CreateContext(m_window.sdl_handle());

            int version = gladLoadGL((GLADloadfunc)SDL3::SDL_GL_GetProcAddress);
            log::info("OpenGL: {}.{}\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
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
            return m_event_handler.handle_events();
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

        sdl::Window& window()
        {
            return m_window;
        }

    private:
        // TODO: implement single instance enforcement
        static inline std::atomic<bool> m_initialized{ false };
        std::once_flag init_flag{};

        sdl::Window m_window{ "Roguelite" };
        sdl::EventHandler m_event_handler{};
    };
}
