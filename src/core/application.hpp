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
            m_main_window = std::make_unique<MainWindow>("Roguelite OpenGL");
        }

        ~Application()
        {
        }

        bool run()
        {
            bool ret{ this->setup() };
            bool bval{ true };

            u64 frame_count{ 0 };
            f32 framerate{ 0.0f };
            f32 elapsed_time{ 0.0f };
            f32 delta_time{ m_timer.delta() };
            std::string sval{ "asdsad" };
            ui::Axis eval{ ui::Horizontal };

            auto gui{ m_main_window->gui() };
            const std::unique_ptr<rl::OpenGLRenderer>& renderer{ m_main_window->glrenderer() };
            gl::InstancedVertexBuffer vbo{ renderer->get_viewport() };
            ui::FormHelper* form{ new ui::FormHelper(gui) };

            auto floating_form_gui = [&] {
                auto dialog = ds::shared{ form->add_dialog(ds::point{ 10, 10 },
                                                           "Nested Diaog Test") };
                form->add_group("Group 1");
                form->add_variable<bool>("checkbox", bval);
                form->add_variable<std::string>("string", sval);

                form->add_group("Dynamic Fields");
                form->add_variable<f32>("fps", framerate)
                    ->set_alignment(ui::TextBox::Alignment::Left);

                form->add_variable<u64>("frame count", frame_count)
                    ->set_alignment(ui::TextBox::Alignment::Center);

                form->add_variable<f32>("elapsed time", elapsed_time)
                    ->set_alignment(ui::TextBox::Alignment::Right);

                form->add_group("Enum");
                form->add_variable<ui::Axis>("Axis", eval, true)
                    ->set_items({
                        "Horizontal",
                        "Vertical",
                        "AAAAAAAAAAAAAAAA",
                        "BBBB",
                        "CCC",
                        "DDDD",
                        "EEEE",
                        "FFFF",
                    });

                form->add_group("Other Group");
                form->add_button("Push Button", []() {
                    std::cout << "Button pressed." << std::endl;
                });

                gui->set_visible(true);
                gui->perform_layout();
                gui->update();

                dialog->center();
            };

            auto full_window_menu = [&] {
                auto layout{ new ui::AdvancedGridLayout({ 0, 0, 0 }, {}, 30) };

                gui->set_layout(layout);
                gui->set_visible(true);

                layout->set_col_stretch(1, 1.0f);

                auto title_label{ new ui::Label{ gui, "GUI Canvas Span Label",
                                                 ui::font::name::sans_bold, 40 } };
                layout->append_row(0);
                auto push_button{ new ui::Button{ gui, "Push Button", ui::Icon::Microscope } };
                layout->append_row(0);
                auto timer_desc_label{ new ui::Label{ gui, "Timer: ", ui::font::name::sans, 32 } };
                layout->append_row(0);
                auto timer_value_label{ new ui::Label{ gui, " ", ui::font::name::mono, 32 } };
                layout->append_row(0);
                auto stats_desc_label{ new ui::Label{ gui, "Stats: ", ui::font::name::sans, 32 } };
                layout->append_row(0);
                auto stats_value_label{ new ui::Label{ gui, "            ", ui::font::name::mono,
                                                       32 } };
                layout->append_row(0);

                push_button->set_tooltip("Microscope Button");
                push_button->set_callback([] {
                    log::warning("Button Pressed Callback Invoked");
                });

                layout->set_anchor(push_button, ui::Anchor(0, layout->row_count() - 1, 1, 1));
                layout->set_anchor(title_label, ui::Anchor{
                                                    1,
                                                    layout->row_count() - 1,
                                                    2,
                                                    1,
                                                    ui::Alignment::Center,
                                                    ui::Alignment::Fill,
                                                });

                layout->append_row(20);
                layout->append_row(0);
                layout->set_anchor(timer_desc_label, ui::Anchor(0, layout->row_count() - 1));
                layout->set_anchor(timer_value_label, ui::Anchor(2, layout->row_count() - 1));
                layout->append_row(10);
                layout->append_row(0);
                layout->set_anchor(stats_desc_label, ui::Anchor(0, layout->row_count() - 1));
                layout->set_anchor(stats_value_label, ui::Anchor(2, layout->row_count() - 1));
                layout->append_row(10);
                layout->append_row(0);

                timer_desc_label->set_tooltip("Timer Label");
                timer_value_label->set_tooltip("Elapsed Time");

                stats_desc_label->set_tooltip("Stats Label");
                stats_value_label->set_tooltip("Average FPS");
                stats_desc_label->set_callback([] {
                    log::warning("Stats callback invoked");
                });

                gui->add_update_callback([&]() {
                    auto&& elapsed_str{ fmt::format("{:.3f} sec", m_timer.elapsed()) };
                    timer_value_label->set_caption(std::move(elapsed_str));
                });

                gui->add_update_callback([&]() {
                    auto&& fps_str{ fmt::to_string(fmt::format("{:.1f} fps", framerate)) };
                    stats_value_label->set_caption(std::move(fps_str));
                });

                gui->update();
                gui->perform_layout();
            };

            floating_form_gui();

            m_timer.reset();
            // vbo.bind_buffers();
            while (!this->should_exit()) [[likely]]
            {
                delta_time = m_timer.delta();

                this->handle_events();
                this->update();
                form->refresh();

                m_main_window->clear();
                gui->draw_all();
                m_main_window->swap_buffers();

                elapsed_time = m_timer.elapsed();
                framerate = ++frame_count / elapsed_time;

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
            return m_event_handler.handle_events(m_main_window);
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

        std::unique_ptr<MainWindow>& main_window()
        {
            return m_main_window;
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
        std::unique_ptr<rl::MainWindow> m_main_window{};
        rl::EventHandler m_event_handler{};
        rl::StateMachine m_fsm{};
    };
}
