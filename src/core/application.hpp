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

            const auto theme{ m_main_window->gui()->theme() };
            const auto label_font{ theme->form_group_font_name };
            const auto label_size{ theme->form_group_font_size };

            auto layout_l{ new ui::NewBoxLayout<Orientation::Vertical>() };
            layout_l->add_widget(new ui::Label{ "A", label_font, label_size });
            layout_l->add_widget(new ui::Label{ "B", label_font, label_size });
            layout_l->add_widget(new ui::Label{ "C", label_font, label_size });

            auto layout_r{ new ui::NewBoxLayout<Orientation::Vertical>() };
            layout_r->add_widget(new ui::Label{ "1", label_font, label_size });
            layout_r->add_widget(new ui::Button{ "ButtonMcButtonFace" });
            layout_r->add_widget(new ui::Label{ "3", label_font, label_size });

            auto horiz_layout{ new ui::NewBoxLayout<Orientation::Horizontal>() };
            horiz_layout->add_layout(layout_l);
            horiz_layout->add_layout(layout_r);

            auto dialog = new ui::Widget{ m_main_window->gui() };
            dialog->set_layout(horiz_layout);
            dialog->set_position({ 100.0f, 100.0f });
            // dialog->set_fixed_height(300);
            // dialog->center();
            // dialog->perform_layout();

            m_timer.reset();
            while (!this->should_exit())
            {
                this->handle_events();
                // dialog->perform_layout();
                // dialog->layout()->apply_layout();
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
        void print_loop_stats(const f32 delta_time)
        {
            const f32 elapsed_time{ m_timer.elapsed() };
            const u64 iterations{ m_timer.tick_count() };
            if (iterations % 60 != 0)
                return;
        }

    private:
        Timer<f32> m_timer{};
        std::unique_ptr<MainWindow> m_main_window{};
        EventHandler m_event_handler{};
    };
}
