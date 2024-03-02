#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "core/assert.hpp"
#include "core/event_handler.hpp"
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "core/state/fsm.hpp"
#include "core/ui/gui.hpp"
#include "core/ui/widgets/button.hpp"
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
        Application(Application&& other) = delete;
        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;
        Application& operator=(Application&& other) = delete;

    public:
        Application()
        {
            this->init_subsystem(Subsystem::All);
            m_main_window = std::make_unique<MainWindow>("Roguelite [OpenGL Window]");
            m_event_handler = EventHandler{ m_main_window };
        }

        ~Application() = default;

        bool run()
        {
            scoped_log();

            bool ret{ this->setup() };
            bool bval{ true };

            u64 frame_count{ 0 };
            f32 framerate{ 0.0f };
            f32 elapsed_time{ 0.0f };
            std::string sval{ "asdsad" };
            ui::Axis eval{ ui::Horizontal };

            const auto gui{ m_main_window->gui() };
            const std::unique_ptr<OpenGLRenderer>& renderer{ m_main_window->glrenderer() };
            gl::InstancedVertexBuffer vbo{ renderer->get_viewport() };
            const auto form{ new ui::FormHelper(gui) };
            auto dialog = ds::shared{ form->add_dialog(ds::point{ 10, 10 }, "Nested Dialog Test") };

            std::string elapsed_str{ fmt::to_string(
                fmt::format("{:0>6.3f} sec", m_timer.elapsed())) };
            std::string fps_str{ fmt::format("{:0>6.3f} fps", framerate) };
            const auto layout{ new ui::AdvancedGridLayout({ 0, 10, 0 }, {}, 30) };

            auto full_window_menu = [&] {
                scoped_log();

                gui->set_layout(layout);
                layout->set_col_stretch(1, 0.5f);

                const auto title_label{ new ui::Label{
                    gui,
                    "GUI Canvas Span Label",
                    ui::Font::Name::SansBold,
                    40,
                } };
                layout->append_row(0);
                const auto push_button{ new ui::Button{
                    gui,
                    "Push Button",
                    ui::Icon::Microscope,
                } };
                layout->append_row(0);
                const auto timer_desc_label{ new ui::Label{
                    gui,
                    "Timer: ",
                    ui::Font::Name::Sans,
                    32,
                } };
                layout->append_row(0);
                const auto timer_value_label{ new ui::Label{
                    gui,
                    "",
                    ui::Font::Name::Mono,
                    32,
                } };
                layout->append_row(0);
                const auto stats_desc_label{ new ui::Label{
                    gui,
                    "Stats: ",
                    ui::Font::Name::Sans,
                    32,
                } };
                layout->append_row(0);
                const auto stats_value_label{ new ui::Label{
                    gui,
                    "",
                    ui::Font::Name::Mono,
                    32,
                } };
                layout->append_row(0);

                gui->add_update_callback([=, &elapsed_str] {
                    timer_value_label->set_text(elapsed_str);
                });

                gui->add_update_callback([=, &fps_str] {
                    stats_value_label->set_text(fps_str);
                });

                push_button->set_tooltip("Microscope Button");
                push_button->set_callback([&] {
                    scoped_log("Push Button Callback Invoked");
                });

                layout->set_anchor(push_button, ui::Anchor(0, layout->row_count() - 1, 1, 1));
                layout->set_anchor(title_label,
                                   ui::Anchor{ 1, layout->row_count() - 1, 2, 1,
                                               ui::Alignment::Center, ui::Alignment::Fill });

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
                stats_desc_label->set_callback([&] {
                    scoped_log("Button Pressed Callback Invoked");
                });
            };

            //////////////////////////////////

            form->add_group("Group 1");
            form->add_variable<bool>("checkbox", bval);
            form->add_variable<std::string>("string", sval);

            form->add_group("Dynamic Fields");
            form->add_variable<f32>("fps", framerate)->set_alignment(ui::TextBox::Alignment::Left);

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
            form->add_button("Push Button", [&] {
                diag_log("Button pressed.\n");
            });

            dialog->center();

            //////////////////////////////////
            // floating_form_gui();
            full_window_menu();

            gui->set_visible(true);
            form->refresh();
            gui->update();
            gui->perform_layout();

            dialog->center();

            m_timer.reset();
            while (!this->should_exit())
            {
                this->handle_events();
                this->update();

                elapsed_time = m_timer.elapsed();
                framerate = static_cast<f32>(++frame_count) / elapsed_time;
                elapsed_str = fmt::format("{:>6.3f} sec", elapsed_time);
                fps_str = fmt::format("{:>6.3f} fps", framerate);
                form->refresh();
                gui->redraw();
                this->render();

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(30ms);
            }

            ret &= this->teardown();
            return ret;
        }

        // vbo.bind_buffers();
        // while() {
        //     vbo.update_buffers(renderer->get_viewport());
        //     vbo.draw_triangles();
        //     if constexpr (io::logging::main_loop)
        //         this->print_loop_stats(delta_time);
        // }

    private:
        void render() const
        {
            m_main_window->render();
        }

        bool setup() const
        {
            return true;
        }

        [[nodiscard]]
        bool quit() const
        {
            return this->teardown();
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
