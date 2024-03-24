#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <cpptrace/cpptrace.hpp>

#include "core/assert.hpp"
#include "core/event_handler.hpp"
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "core/state/fsm.hpp"
#include "core/ui/gui.hpp"
#include "graphics/gl/instanced_buffer.hpp"
#include "sdl/defs.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_init.h>
SDL_C_LIB_END

namespace rl {
    class Application
    {
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
            m_main_window = std::make_unique<MainWindow>("Roguelite [OpenGL Window]");
            m_event_handler = EventHandler{ m_main_window };
        }

        Application(Application&& other) = delete;
        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;
        Application& operator=(Application&& other) = delete;

        ~Application() = default;

    public:
        bool run()
        {
            bool ret{ this->setup() };

            auto dialog = new ui::ScrollableDialog{
                m_main_window->gui(),
                "asdfasdf",
                {
                    { new ui::Label{ "A" }, new ui::Label{ "1" } },
                    { new ui::Label{ "B" }, new ui::Label{ "2" } },
                    { new ui::Label{ "C" }, new ui::Label{ "3" } },
                    { new ui::Label{ "D" }, new ui::Label{ "4" } },
                }
            };

            dialog->set_position(ds::point{ 10.0f, 10.0f });
            dialog->set_fixed_height(300);
            dialog->center();

            m_timer.reset();
            while (!this->should_exit())
            {
                this->handle_events();
                this->update();
                this->render();

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(30ms);
            }

            ret &= this->teardown();
            return ret;
        }

        void render() const
        {
            m_main_window->render();
        }

        bool setup() const
        {
            return true;
        }

        bool teardown() const
        {
            return true;
        }

        bool handle_events()
        {
            return m_event_handler.handle_events(m_main_window);
        }

        void update() const
        {
        }

        [[nodiscard]]
        bool should_exit() const
        {
            return m_event_handler.quit_triggered();
        }

        [[nodiscard]]
        bool quit() const
        {
            return this->teardown();
        }

        bool init_subsystem(const Subsystem::ID flags) const
        {
            const i32 result = SDL3::SDL_Init(flags);
            runtime_assert(result == 0, "failed to init subsystem");
            return result == 0;
        }

        std::unique_ptr<MainWindow>& main_window()
        {
            return m_main_window;
        }

        Application& sdl()
        {
            return *this;
        }

    private:
        static std::string name()
        {
            return "Application";
        }

        void print_loop_stats(const f32 delta_time)
        {
            f32 elapsed_time{ m_timer.elapsed() };
            u64 iterations{ m_timer.tick_count() };
            if (iterations % 60 != 0)
                return;

            scoped_log(
                " {:>14.6f} s || {:>10L} u ][ {:>10.4f} ms | {:>10.4f} fps ][ {:>10.4f} avg fps ]",
                elapsed_time, iterations, delta_time * 1000.0f, 1.0f / delta_time,
                static_cast<f32>(iterations) / elapsed_time);
        }

    private:
        Timer<f32> m_timer{};
        std::unique_ptr<MainWindow> m_main_window{};
        EventHandler m_event_handler{};
    };
}
