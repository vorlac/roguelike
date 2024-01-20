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
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "core/state/fsm.hpp"
#include "core/state/states.hpp"
#include "core/ui/button.hpp"
#include "core/ui/gui.hpp"
#include "graphics/gl/instanced_buffer.hpp"
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
            m_window = std::make_unique<MainWindow>("Roguelite OpenGL");
        }

        ~Application()
        {
        }

        void old_gui()
        {
            // auto layout{ new ui::AdvancedGridLayout({ 0, 0, 0 }, {}, 30) };
            // auto gui{ m_window->gui() };
            //
            // gui->set_layout(layout);
            // gui->set_visible(true);
            //
            // layout->set_col_stretch(1, 1.0f);
            //
            // auto title_label{ new ui::Label{ gui, "GUI Canvas Span Label",
            //                                  ui::font::name::sans_bold, 40 } };
            // layout->append_row(0);
            // auto push_button{ new ui::Button{ gui, "Push Button", ui::Icon::Microscope } };
            // layout->append_row(0);
            // auto timer_desc_label{ new ui::Label{ gui, "Timer: ", ui::font::name::sans, 32 } };
            // layout->append_row(0);
            // auto timer_value_label{ new ui::Label{ gui, "            ", ui::font::name::mono, 32
            // } }; layout->append_row(0); auto stats_desc_label{ new ui::Label{ gui, "Stats: ",
            // ui::font::name::sans, 32 } }; layout->append_row(0); auto stats_value_label{ new
            // ui::Label{ gui, "            ", ui::font::name::mono, 32 } }; layout->append_row(0);
            //
            // push_button->set_tooltip("Microscope Button");
            // push_button->set_callback([] {
            //     log::warning("Button Pressed Callback Invoked");
            // });
            //
            // layout->set_anchor(push_button, ui::Anchor(0, layout->row_count() - 1, 1, 1));
            // layout->set_anchor(title_label, ui::Anchor{
            //                                     1,
            //                                     layout->row_count() - 1,
            //                                     2,
            //                                     1,
            //                                     ui::Alignment::Center,
            //                                     ui::Alignment::Fill,
            //                                 });
            //
            // layout->append_row(20);
            // layout->append_row(0);
            // layout->set_anchor(timer_desc_label, ui::Anchor(0, layout->row_count() - 1));
            // layout->set_anchor(timer_value_label, ui::Anchor(2, layout->row_count() - 1));
            // layout->append_row(10);
            // layout->append_row(0);
            // layout->set_anchor(stats_desc_label, ui::Anchor(0, layout->row_count() - 1));
            // layout->set_anchor(stats_value_label, ui::Anchor(2, layout->row_count() - 1));
            // layout->append_row(10);
            // layout->append_row(0);
            //
            // timer_desc_label->set_tooltip("Timer Label");
            // timer_value_label->set_tooltip("Elapsed Time");
            //
            // stats_desc_label->set_tooltip("Stats Label");
            // stats_value_label->set_tooltip("Average FPS");
            // stats_desc_label->set_callback([] {
            //     log::warning("Stats callback invoked");
            // });
            //
            // gui->add_update_callback([&]() {
            //     auto&& elapsed_str{ fmt::format("{:.3f} sec", m_timer.elapsed()) };
            //     timer_value_label->set_caption(std::move(elapsed_str));
            // });
            //
            // gui->add_update_callback([&]() {
            //     auto&& fps_str{ fmt::to_string(fmt::format("{:.1f} fps", fps)) };
            //     stats_value_label->set_caption(std::move(fps_str));
            // });
            //
            // gui->update();
            // gui->perform_layout();
        }

        void new_gui()
        {
            // auto gui{ m_window->gui() };
            // ui::FormHelper* form{ new ui::FormHelper(gui) };
            // ds::shared<ui::Dialog> dialog{ gui->add_window({ 10, 10 }, "Form helper example") };
            // form->add_group("Basic types");
            // form->add_variable("bool", true);
            // form->add_variable("string", "asdfg");
            //
            // form->add_group("Validating fields");
            // form->add_variable("int", 123);
            // form->add_variable("float", 1.23f);
            // form->add_variable("double", 3.14);
            //
            // form->add_group("Complex types");
            // form->add_variable("Enumeration", ui::Horizontal, true)
            //     ->setItems({ "Item 1", "Item 2", "Item 3" });
            //
            // form->add_group("Other widgets");
            // form->add_button("A button", []() {
            //     std::cout << "Button pressed." << std::endl;
            // });
            //
            // gui->set_visible(true);
            // gui->perform_layout();
            // gui->center();
        }

        bool run()
        {
            bool ret{ this->setup() };

            f32 fps{ 0 };
            u64 frame_count{ 0 };
            f32 delta_time{ m_timer.delta() };

            const std::unique_ptr<rl::OpenGLRenderer>& renderer{ m_window->renderer() };
            gl::InstancedVertexBuffer vbo{ renderer->get_viewport() };

            bool bval{ true };
            i32 ival{ true };
            f32 fval{ true };
            f64 dval{ true };
            std::string sval{ "asdsad" };
            ui::Axis eval{ ui::Horizontal };
            auto gui{ m_window->gui() };
            ui::FormHelper* form{ new ui::FormHelper(gui) };
            ds::shared<ui::Dialog> dialog{ form->add_dialog({ 10, 10 }, "Nested Diaog Test") };
            form->add_group("Basic types");
            form->add_variable<bool>("bool", bval);
            form->add_variable<std::string>("string", sval);

            form->add_group("Validating fields");
            form->add_variable<i32>("int", ival);
            form->add_variable<f32>("float", fval);
            form->add_variable<f64>("double", dval);

            form->add_group("Complex types");
            form->add_variable<ui::Axis>("Enumeration", eval, true)
                ->set_items({ "Horizontal", "Vertical" });

            form->add_group("Other widgets");
            form->add_button("A button", []() {
                std::cout << "Button pressed." << std::endl;
            });

            gui->set_visible(true);
            gui->perform_layout();
            dialog->center();
            // vbo.bind_buffers();

            m_timer.reset();
            while (!this->should_exit()) [[likely]]
            {
                delta_time = m_timer.delta();

                this->handle_events();
                this->update();

                m_window->clear();
                gui->draw_all();
                m_window->swap_buffers();

                fps = ++frame_count / m_timer.elapsed();

                // vbo.update_buffers(renderer->get_viewport());
                // vbo.draw_triangles();

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

        std::unique_ptr<MainWindow>& window()
        {
            return m_window;
        }

        rl::Application& sdl()
        {
            return *this;
        }

    private:
        Application(const rl::Application& other) = delete;
        Application(rl::Application&& other) = delete;
        Application& operator=(const rl::Application& other) = delete;
        Application& operator=(rl::Application&& other) = delete;

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
        std::unique_ptr<rl::MainWindow> m_window{};
        rl::EventHandler m_event_handler{};
        rl::StateMachine m_fsm{};
    };
}
