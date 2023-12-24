#pragma once

#include <array>
#include <atomic>
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "core/assert.hpp"
#include "core/event_handler.hpp"
#include "core/renderer.hpp"
#include "core/state/fsm.hpp"
#include "core/state/states.hpp"
#include "core/ui/label.hpp"
#include "core/window.hpp"
#include "gl/instanced_buffer.hpp"
#include "sdl/defs.hpp"
#include "utils/crtp.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_init.h>
SDL_C_LIB_END

namespace rl {
    class Application
    {
    private:
        Application(const rl::Application& other) = delete;
        Application(rl::Application&& other) = delete;
        Application& operator=(const rl::Application& other) = delete;
        Application& operator=(rl::Application&& other) = delete;

    public:
        struct Subsystem
        {
            enum ID : u16_fast {
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
        };

    public:
        Application()
        {
            this->init_subsystem(Subsystem::All);
            m_window = std::make_unique<Window>("Roguelite OpenGL");
        }

        ~Application()
        {
        }

        bool run()
        {
            bool ret{ this->setup() };
            f32 delta_time{ m_timer.delta() };

            const std::unique_ptr<rl::Renderer>& renderer{ m_window->renderer() };
            gl::InstancedVertexBuffer vbo{ renderer->get_viewport() };

            ui::label* label = new ui::label(m_window.get(), "TEST");
            label->set_callback([] {
                log::warning("Button Pressed");
            });

            vbo.bind_buffers();
            while (!this->should_exit()) [[likely]]
            {
                delta_time = m_timer.delta();

                this->handle_events();
                this->update();

                renderer->clear();
                vbo.update_buffers(renderer->get_viewport());
                vbo.draw_triangles();
                m_window->swap_buffers();

                if constexpr (io::logging::main_loop)
                    this->print_loop_stats(delta_time);
            }

            ret &= this->teardown();
            return ret;
        }

    private:
        inline bool setup()
        {
            return true;
        }

        bool quit()

        {
            return this->teardown();
        }

        inline bool teardown()
        {
            return true;
        }

        inline bool handle_events()
        {
            return m_event_handler.handle_events(m_window);
        }

        inline bool update()
        {
            return true;
        }

        inline bool should_exit() const
        {
            return m_event_handler.quit_triggered();
        }

        bool init_subsystem(Subsystem::ID flags)
        {
            i32 result = SDL3::SDL_Init(flags);
            runtime_assert(result == 0, "failed to init subsystem");
            return result == 0;
        }

        std::unique_ptr<Window>& window()
        {
            return m_window;
        }

        rl::Application& sdl()
        {
            return *this;
        }

    private:
        inline void print_loop_stats(f32 delta_time)
        {
            f32 elapsed_time{ m_timer.elapsed() };
            u64 iterations{ m_timer.tick_count() };
            if (iterations % 60 != 0)
                return;

            log::debug(
                " {:>14.6f} s || {:>10L} u ][ {:>10.4f} ms | {:>10.4f} fps ][ {:>10.4f} avg fps ]",
                elapsed_time,                // elapsed time (seconds)
                iterations,                  // loop iterations
                delta_time * 1000.0f,        // delta time (ms)
                1.0f / delta_time,           // current fps
                iterations / elapsed_time);  // avg fps
        }

    private:
        rl::Timer<f32> m_timer{};
        std::unique_ptr<rl::Window> m_window{};
        rl::EventHandler m_event_handler{};
        rl::StateMachine m_fsm{};
    };
}
