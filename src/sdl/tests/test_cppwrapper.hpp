
namespace rl::sdl::test {

    int render_test_1()
    {
        SDL3::SDL_Event event;
        while (SDL3::SDL_PollEvent(&event))
            if (event.type == SDL3::SDL_EVENT_QUIT || (event.type == SDL3::SDL_EVENT_KEY_DOWN &&
                                                       (event.key.keysym.sym == SDL3::SDLK_ESCAPE ||
                                                        event.key.keysym.sym == SDL3::SDLK_q)))
                return 0;

        // Note we fill with transparent color, not black
        m_renderer.set_draw_color({ 0, 0, 0, 0 });

        // Fill base texture with sprite texture
        m_renderer.set_target(m_target1);
        m_renderer.clear();
        m_renderer.copy(m_sprite);

        // Repeat several cycles of flip-flop tiling
        for (int i = 0; i < 4; i++)
        {
            m_renderer.set_target(m_target2);
            m_renderer.clear();

            m_renderer.copy(m_target1, ds::rect<i32>::null(),
                            ds::rect<i32>{ 0, 0, 512 / 2, 512 / 2 },
                            cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

            m_renderer.copy(m_target1, ds::rect<i32>::null(),
                            ds::rect<i32>{ 512 / 2, 0, 512 / 2, 512 / 2 },
                            cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

            m_renderer.copy(m_target1, ds::rect<i32>::null(),
                            ds::rect<i32>{ 0, 512 / 2, 512 / 2, 512 / 2 },
                            cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

            m_renderer.copy(m_target1, ds::rect<i32>::null(),
                            ds::rect<i32>{ 512 / 2, 512 / 2, 512 / 2, 512 / 2 },
                            cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

            // Swap textures to copy recursively
            std::swap(m_target1, m_target2);
        }

        // Draw result to screen
        m_renderer.set_target();
        m_renderer.clear();
        // 640, 480
        m_renderer.copy(m_target1, ds::rect<i32>::null(),
                        ds::rect<i32>{ (640 - 480) / 2, 0, 480, 480 },
                        cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

        m_renderer.present();

        // Frame limiter
        SDL3::SDL_Delay(1);
        return true;
    }

    int render_test_1()
    {
        PixelInspector pixels(320, 240, 4);

        {
            // Clear, draw color
            m_renderer.set_draw_color({ 1, 2, 3 });

            sdl::color&& c = m_renderer.get_draw_color();
            runtime_assert(c.r == 1 && c.g == 2 && c.b == 3 && c.a == 255, "test failed");

            m_renderer.clear();
            pixels.Retrieve(m_renderer);

            auto res = pixels.Test(0, 0, 1, 2, 3);
            runtime_assert(res, "test failed");

            m_renderer.present();
            SDL3::SDL_Delay(1000);
        }

        {
            // Draw points
            m_renderer.set_draw_color(sdl::Color{ 0, 0, 0 });
            m_renderer.clear();

            m_renderer.set_draw_color(sdl::Color{ 255, 128, 0 });
            m_renderer.draw_point({ 10, 10 });

            m_renderer.set_draw_color(sdl::Color{ 0, 255, 128 });
            m_renderer.draw_point(ds::point<i32>{ 20, 20 });

            m_renderer.set_draw_color(sdl::Color{ 128, 0, 255 });
            std::vector<ds::point<f32>> points = { { 30, 30 } };
            m_renderer.draw_points(points);
            pixels.Retrieve(m_renderer);

            auto res1 = pixels.Test3x3(10, 10, 0x020, 255, 128, 0);
            auto res2 = pixels.Test3x3(20, 20, 0x020, 0, 255, 128);
            auto res3 = pixels.Test3x3(30, 30, 0x020, 128, 0, 255);
            runtime_assert(res1, "test failed");
            runtime_assert(res2, "test failed");
            runtime_assert(res3, "test failed");

            m_renderer.present();
            SDL3::SDL_Delay(1000);
        }

        {
            // Draw lines
            m_renderer.set_draw_color(sdl::Color{ 0, 0, 0 });
            m_renderer.clear();

            m_renderer.set_draw_color(sdl::Color{ 255, 128, 0 });
            m_renderer.draw_line({ 10, 10 }, { 10, 50 });

            m_renderer.set_draw_color(sdl::Color{ 0, 255, 128 });
            m_renderer.draw_line(ds::point<i32>{ 20, 10 }, ds::point<i32>{ 20, 50 });

            m_renderer.set_draw_color(sdl::Color{ 128, 0, 255 });
            std::vector<ds::point<f32>> points = { { 30, 10 }, { 30, 50 } };
            m_renderer.draw_lines(points);

            pixels.Retrieve(m_renderer);

            auto res1 = pixels.Test3x3(10, 20, 0x222, 255, 128, 0);
            auto res2 = pixels.Test3x3(20, 20, 0x222, 0, 255, 128);
            auto res3 = pixels.Test3x3(30, 20, 0x222, 128, 0, 255);
            runtime_assert(res1, "test failed");
            runtime_assert(res2, "test failed");
            runtime_assert(res3, "test failed");

            m_renderer.present();
            SDL3::SDL_Delay(1000);
        }
    }

    int render_test_colors(sdl::Window& Window)
    {
        double delta{ 0 };
        u64 loop_count{ 0 };
        srand((u32)time(nullptr));

        auto& window{ m_sdl.window() };
        auto renderer{ window.renderer() };
        u8 a = 0;
        sdl::Color color{ 0, 0, 0, 0 };
        sdl::Application::timer_t timer{};
        while (!quit_requested()) [[unlikely]]
        {
            renderer->set_draw_color(color);

            renderer->clear();
            renderer->present();

            if (++loop_count % 960 == 0) [[unlikely]]
            {
                const double elapsed_ms = timer.elapsed();
                const double avg_dlt{ (elapsed_ms * 1000.0) / cast::to<f64>(loop_count) };
                const double avg_ups{ loop_count / (elapsed_ms) };
                printf(fmt::format("[prev_dt={:<6.4f}ms] [avg_dlt={:<6.4f}ms] [avg_ups={:<6.4f}]\n",
                                   timer.delta(), avg_ups, avg_dlt)
                           .data());
                color = {
                    rand() % 128,
                    rand() % 128,
                    rand() % 128,
                    ++a + 127 % 255,
                };
            }

            timer.delta();
        }
    }
}
