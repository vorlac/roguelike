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
#include "graphics/gl/instanced_buffer.hpp"
#include "sdl/defs.hpp"
#include "ui/gui.hpp"
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

            constexpr auto font_size{ 20.0f };
            constexpr auto alignment{ Align::VMiddle | Align::HCenter };

            const auto layout_h1{ new ui::BoxLayout<Alignment::Horizontal>("Letters H1") };
            layout_h1->add_widget(new ui::Label{ "A", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "B", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "C", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "D", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "E", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "F", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "G", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "H", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "I", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "J", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "K", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "L", font_size, alignment });
            layout_h1->add_widget(new ui::Label{ "M", font_size, alignment });

            const auto layout_h2{ new ui::BoxLayout<Alignment::Horizontal>("Letters H2") };
            layout_h2->add_widget(new ui::Label{ "N", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "O", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "P", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "Q", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "R", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "S", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "T", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "U", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "V", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "W", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "X", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "Y", font_size, alignment });
            layout_h2->add_widget(new ui::Label{ "Z", font_size, alignment });

            const auto layout_v1{ new ui::BoxLayout<Alignment::Vertical>("Nums V1") };
            layout_v1->add_widget(new ui::Label{ "123", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v2{ new ui::BoxLayout<Alignment::Vertical>("Nums V1") };

            const auto layout_v3{ new ui::BoxLayout<Alignment::Vertical>("Nums V2") };
            layout_v3->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto horiz_nums_layout{ new ui::BoxLayout<Alignment::Horizontal>("ABC123 Horiz") };
            horiz_nums_layout->add_nested_layout(layout_v1);
            horiz_nums_layout->add_nested_layout(layout_v2);
            horiz_nums_layout->add_nested_layout(layout_v3);

            const auto horiz_alph_layout{ new ui::BoxLayout<Alignment::Vertical>("ABC Horiz") };
            horiz_alph_layout->add_nested_layout(layout_h1);
            horiz_alph_layout->add_nested_layout(layout_h2);

            auto layout_v_letters{ new ui::BoxLayout<Alignment::Vertical>("Letters H2") };
            layout_v_letters->add_nested_layout(horiz_nums_layout);
            layout_v_letters->add_nested_layout(horiz_alph_layout);

            m_main_window->gui()->assign_layout(layout_v_letters);

            m_timer.reset();
            while (!this->should_exit()) {
                this->handle_events();
                this->update();
                this->render();
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
