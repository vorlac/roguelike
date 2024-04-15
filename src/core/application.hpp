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

            const auto layout_h3{ new ui::BoxLayout<Alignment::Horizontal>("Letters H3") };
            layout_h3->add_widget(new ui::Label{ "A", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "B", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "C", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "D", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "E", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "F", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "G", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "H", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "I", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "J", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "K", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "L", font_size, alignment });
            layout_h3->add_widget(new ui::Label{ "M", font_size, alignment });

            const auto layout_h4{ new ui::BoxLayout<Alignment::Horizontal>("Letters H4") };
            layout_h4->add_widget(new ui::Label{ "N", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "O", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "P", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "Q", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "R", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "S", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "T", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "U", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "V", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "W", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "X", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "Y", font_size, alignment });
            layout_h4->add_widget(new ui::Label{ "Z", font_size, alignment });

            const auto layout_v1{ new ui::BoxLayout<Alignment::Vertical>("Nums V1") };
            layout_v1->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v1->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v2{ new ui::BoxLayout<Alignment::Vertical>("Nums V2") };
            layout_v2->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v2->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v2->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v2->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v2->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v3{ new ui::BoxLayout<Alignment::Vertical>("Nums V3") };
            layout_v3->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v3->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v4{ new ui::BoxLayout<Alignment::Vertical>("Nums V4") };
            layout_v4->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v4->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v4->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v4->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v4->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v5{ new ui::BoxLayout<Alignment::Vertical>("Nums V5") };
            layout_v5->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v5->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v5->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v5->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v5->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v6{ new ui::BoxLayout<Alignment::Vertical>("Nums V6") };
            layout_v6->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v6->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v6->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v6->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v6->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v7{ new ui::BoxLayout<Alignment::Vertical>("Nums V7") };
            layout_v7->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v7->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v7->add_widget(new ui::Label{ "3", font_size, alignment });

            const auto layout_v8{ new ui::BoxLayout<Alignment::Vertical>("Nums V8") };
            layout_v8->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v8->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v8->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v8->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v8->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v8->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v8->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v9{ new ui::BoxLayout<Alignment::Vertical>("Nums V9") };
            layout_v9->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v9->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v9->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v9->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v9->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v10{ new ui::BoxLayout<Alignment::Vertical>("Nums V10") };
            layout_v10->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v10->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v10->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v10->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v10->add_widget(new ui::Label{ "5", font_size, alignment });
            layout_v10->add_widget(new ui::Label{ "6", font_size, alignment });
            layout_v10->add_widget(new ui::Label{ "7", font_size, alignment });

            const auto layout_v11{ new ui::BoxLayout<Alignment::Vertical>("Nums V11") };
            layout_v11->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v11->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v11->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v11->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v11->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v12{ new ui::BoxLayout<Alignment::Vertical>("Nums V12") };
            layout_v12->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "5", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v12->add_widget(new ui::Label{ "5", font_size, alignment });

            const auto layout_v13{ new ui::BoxLayout<Alignment::Vertical>("Nums V13") };
            layout_v13->add_widget(new ui::Label{ "0", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "1", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "2", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "3", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "4", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "5", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "6", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "7", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "8", font_size, alignment });
            layout_v13->add_widget(new ui::Label{ "9", font_size, alignment });

            const auto horiz_nums_layout{ new ui::BoxLayout<Alignment::Horizontal>("ABC123 Horiz") };
            horiz_nums_layout->add_nested_layout(layout_v1);
            horiz_nums_layout->add_nested_layout(layout_v2);
            horiz_nums_layout->add_nested_layout(layout_v3);
            horiz_nums_layout->add_nested_layout(layout_v4);
            horiz_nums_layout->add_nested_layout(layout_v5);
            horiz_nums_layout->add_nested_layout(layout_v6);
            horiz_nums_layout->add_nested_layout(layout_v7);
            horiz_nums_layout->add_nested_layout(layout_v8);
            horiz_nums_layout->add_nested_layout(layout_v9);
            horiz_nums_layout->add_nested_layout(layout_v10);
            horiz_nums_layout->add_nested_layout(layout_v11);
            horiz_nums_layout->add_nested_layout(layout_v12);
            horiz_nums_layout->add_nested_layout(layout_v13);

            const auto horiz_alph_layout{ new ui::BoxLayout<Alignment::Vertical>("ABC Horiz") };
            horiz_alph_layout->add_nested_layout(layout_h1);
            horiz_alph_layout->add_nested_layout(layout_h2);
            horiz_alph_layout->add_nested_layout(layout_h3);
            horiz_alph_layout->add_nested_layout(layout_h4);

            auto layout_v_letters{ new ui::BoxLayout<Alignment::Vertical>("Letters H2") };
            layout_v_letters->add_nested_layout(horiz_nums_layout);
            layout_v_letters->add_nested_layout(horiz_alph_layout);

            m_main_window->gui()->assign_layout(layout_v_letters);

            m_timer.reset();
            while (!this->should_exit())
            {
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
