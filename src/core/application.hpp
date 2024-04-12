#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "core/assert.hpp"
#include "core/event_handler.hpp"
#include "core/main_window.hpp"
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

        ~Application()
        {
            SDL3::SDL_Quit();
        }

    public:
        bool run()
        {
            bool ret{ this->setup() };

            const auto layout_l{ new ui::BoxLayout<Alignment::Vertical>("ABC Vertical") };
            layout_l->add_widget(new ui::Label{ "A" });
            layout_l->add_widget(new ui::Label{ "B" });
            layout_l->add_widget(new ui::Label{ "C" });
            // layout_l->set_margins(ds::margin<f32>::zero(), ds::margin<f32>::zero());

            const auto layout_r{ new ui::BoxLayout<Alignment::Vertical>("123 Vertical") };
            layout_r->add_widget(new ui::Label{ "1" });
            layout_r->add_widget(new ui::Label{ "2" });
            layout_r->add_widget(new ui::Label{ "3" });
            // layout_r->set_margins(ds::margin<f32>::zero(), ds::margin<f32>::zero());

            const auto horiz_layout{ new ui::BoxLayout<Alignment::Horizontal>("ABC123 Horiz") };
            horiz_layout->add_nested_layout(layout_l);
            horiz_layout->add_nested_layout(layout_r);
            // horiz_layout->set_margins(ds::margin<f32>::zero(), ds::margin<f32>::init(5.0f));

            m_main_window->gui()->assign_layout(horiz_layout);

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
            m_main_window->gui()->perform_layout();
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
            const i32 result{ SDL3::SDL_Init(flags) };
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
        Timer<f32> m_timer{};
        std::unique_ptr<MainWindow> m_main_window{};
        EventHandler m_event_handler{};
    };
}
