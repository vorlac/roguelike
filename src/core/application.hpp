#pragma once

#include "core/assert.hpp"
#include "core/event_handler.hpp"
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "gfx/gl/instanced_buffer.hpp"
#include "ui/gui.hpp"
#include "utils/debug.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"
#include "utils/sdl_defs.hpp"
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
                Timer = SDL_INIT_TIMER,
                Audio = SDL_INIT_AUDIO,
                Video = SDL_INIT_VIDEO,
                Joystick = SDL_INIT_JOYSTICK,
                Haptic = SDL_INIT_HAPTIC,
                Gamepad = SDL_INIT_GAMEPAD,
                Events = SDL_INIT_EVENTS,
                Sensor = SDL_INIT_SENSOR,
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
            [[maybe_unused]] bool status{ this->init_subsystem(Subsystem::All) };
            debug_assert(status, "failed to init SDL subsystems");
            m_main_window = std::make_unique<MainWindow>("Roguelite [OpenGL Window]");
            m_event_handler = EventHandler{ m_main_window };
        }

        ~Application()
        {
            SDL3::SDL_Quit();
        }

        enum class UITest {
            CanvasLayout,
            ScrollDialog,
        };

        i32 run()
        {
            bool ret{ this->setup() };

            constexpr auto test_case{ UITest::ScrollDialog };
            if constexpr (test_case == UITest::CanvasLayout) {
                constexpr auto font_size{ 20.0f };
                constexpr auto alignment{ Align::VMiddle | Align::HCenter };

                const auto layout_abc_h1{
                    new ui::BoxLayout<Alignment::Horizontal>(
                        "A=>M Inner Horiz",
                        {
                            new ui::Label{ "A", font_size, alignment },
                            new ui::Label{ "B", font_size, alignment },
                            new ui::Label{ "C", font_size, alignment },
                            new ui::Label{ "D", font_size, alignment },
                            new ui::Label{ "E", font_size, alignment },
                            new ui::Label{ "F", font_size, alignment },
                            new ui::Label{ "G", font_size, alignment },
                            new ui::Label{ "H", font_size, alignment },
                            new ui::Label{ "I", font_size, alignment },
                            new ui::Label{ "J", font_size, alignment },
                            new ui::Label{ "K", font_size, alignment },
                            new ui::Label{ "L", font_size, alignment },
                            new ui::Label{ "M", font_size, alignment },
                        })
                };

                const auto layout_abc_h2{
                    new ui::BoxLayout<Alignment::Horizontal>(
                        "N=>Z Inner Horiz",
                        {
                            new ui::Label{ "N", font_size, alignment },
                            new ui::Label{ "O", font_size, alignment },
                            new ui::Label{ "P", font_size, alignment },
                            new ui::Label{ "Q", font_size, alignment },
                            new ui::Label{ "R", font_size, alignment },
                            new ui::Label{ "S", font_size, alignment },
                            new ui::Label{ "T", font_size, alignment },
                            new ui::Label{ "U", font_size, alignment },
                            new ui::Label{ "V", font_size, alignment },
                            new ui::Label{ "W", font_size, alignment },
                            new ui::Label{ "X", font_size, alignment },
                            new ui::Label{ "Y", font_size, alignment },
                            new ui::Label{ "Z", font_size, alignment },
                        })
                };

                const auto label{ new ui::Label{ "5x Expansion", font_size, alignment } };
                label->set_expansion(5.0f);
                const auto layout_num_v1{
                    new ui::BoxLayout<Alignment::Vertical>(
                        "Nums1 Inner Vert",
                        {
                            new ui::Button{ "Button", ui::Icon::ID::Bong },
                            new ui::Label{ "2", font_size, alignment },
                            label,
                            new ui::Label{ "4", font_size, alignment },
                            new ui::Label{ "5", font_size, alignment },
                        })
                };

                const auto layout_num_v2{
                    new ui::BoxLayout<Alignment::Vertical>(
                        "Nums2 Inner Vert",
                        {
                            new ui::CheckBox{ "Checkbox ASDFGHJKL" },
                            new ui::Label{ "2", font_size, alignment },
                            new ui::Label{ "3", font_size, alignment },
                            new ui::Label{ "4", font_size, alignment },
                            new ui::Label{ "5", font_size, alignment },
                        })
                };

                const auto layout_nums_horiz_outer{
                    new ui::BoxLayout<Alignment::Horizontal>(
                        "Nums Outer Horiz",
                        {
                            layout_num_v1,
                            layout_num_v2,
                        })
                };

                const auto layout_abc_vert_nested{
                    new ui::BoxLayout<Alignment::Vertical>(
                        "ABC Nested Vert",
                        {
                            layout_abc_h1,
                            layout_abc_h2,
                        })

                };

                const auto layout_canvas_vert{ new ui::BoxLayout<Alignment::Vertical>("Top Level Vert") };
                layout_canvas_vert->set_size_policy(SizePolicy::Maximum);
                layout_canvas_vert->add_nested_layout(layout_nums_horiz_outer);
                layout_canvas_vert->add_nested_layout(layout_abc_vert_nested);
                m_main_window->gui()->assign_layout(layout_canvas_vert);
            }
            else if constexpr (test_case == UITest::ScrollDialog) {
                const auto dialog{
                    new ui::ScrollableDialog{
                        "Dialog Title", ds::dims{ 600, 800 } },
                };
                m_main_window->gui()->add_child(dialog);
                dialog->center();
            }

            m_timer.reset();
            while (!this->should_exit()) {
                this->handle_events();
                this->update();
                this->render();

                if constexpr (debug::core::loop_timing_stats)
                    this->print_loop_stats(m_timer.delta());
                if constexpr (debug::core::loop_throttling) {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(30ms);
                }
            }

            ret &= this->teardown();
            return ret ? 0 : 1;
        }

        void render() const
        {
            m_main_window->render();
        }

        [[nodiscard]]
        bool setup() const
        {
            return true;
        }

        [[nodiscard]]
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

        [[nodiscard]]
        bool init_subsystem(const Subsystem::ID flags) const
        {
            const i32 result{ SDL3::SDL_Init(flags) };
            debug_assert(result == 0, "failed to init subsystem");
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
            f32 elapsed_time{ m_timer.elapsed() };
            u64 iterations{ m_timer.tick_count() };
            if (iterations % 2400 != 0)
                return;

            log::debug(
                " {:>14.6f} s || {:>10L} u ][ {:>10.4f} ms | {:>10.4f} fps ][ {:>10.4f} avg fps ]",
                elapsed_time,                                  // elapsed time (seconds)
                iterations,                                    // loop iterations
                delta_time * 1000.0f,                          // delta time (ms)
                1.0f / delta_time,                             // current fps
                static_cast<f32>(iterations) / elapsed_time);  // avg fps
        }

    private:
        Timer<f32> m_timer{};
        std::unique_ptr<MainWindow> m_main_window{};
        EventHandler m_event_handler{};
    };
}
