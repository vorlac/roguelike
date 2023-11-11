#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "core/numeric_types.hpp"
#include "sdl/events.hpp"
#include "sdl/renderer.hpp"
#include "sdl/time.hpp"
#include "sdl/window.hpp"

namespace SDL3 {
#include <SDL3/SDL_blendmode.h>
}

namespace rl::sdl {
    /*class PixelInspector
    {
    private:
        std::vector<unsigned char> pixels_;
        int width_;
        int height_;
        int bpp_;

    public:
        PixelInspector(int width, int height, int bpp)
            : pixels_(width * height * bpp, 0)
            , width_(width)
            , height_(height)
            , bpp_(bpp)
        {
        }

        void Retrieve(sdl::renderer& renderer)
        {
            renderer.read_pixels({ 0, 0, width_, height_ }, SDL3::SDL_PIXELFORMAT_ARGB8888,
                                 pixels_.data(), width_ * bpp_);
        }

        bool Test(int x, int y, int r, int g, int b, int a = -1)
        {
            int offset = (x + y * width_) * bpp_;

            if (b >= 0 && pixels_[offset] != b)
                return false;
            if (g >= 0 && pixels_[offset + 1] != g)
                return false;
            if (r >= 0 && pixels_[offset + 2] != r)
                return false;
            if (a >= 0 && pixels_[offset + 3] != a)
                return false;

            return true;
        }

        bool Test3x3(int x, int y, int mask, int r, int g, int b, int a = -1)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    bool maskbit = !!(mask & (1 << ((1 - dx) + (1 - dy) * 4)));
                    if (Test(x + dx, y + dy, r, g, b, a) != maskbit)
                        return false;
                }
            }
            return true;
        }
    };*/

    class application
    {
    public:
        enum Subsystem : u16_fast {
            Timer = SDL3::SDL_INIT_TIMER,
            Audio = SDL3::SDL_INIT_AUDIO,
            Video = SDL3::SDL_INIT_VIDEO,
            Joystick = SDL3::SDL_INIT_JOYSTICK,
            Haptic = SDL3::SDL_INIT_HAPTIC,
            Gamepad = SDL3::SDL_INIT_GAMEPAD,
            Events = SDL3::SDL_INIT_EVENTS,
            Sensor = SDL3::SDL_INIT_SENSOR,

            Count = Sensor,

            All = SDL3::SDL_INIT_TIMER |     //
                  SDL3::SDL_INIT_AUDIO |     //
                  SDL3::SDL_INIT_VIDEO |     //
                  SDL3::SDL_INIT_JOYSTICK |  //
                  SDL3::SDL_INIT_HAPTIC |    //
                  SDL3::SDL_INIT_GAMEPAD |   //
                  SDL3::SDL_INIT_EVENTS |    //
                  SDL3::SDL_INIT_SENSOR
        };

        using timer_t = sdl::perftimer<double, sdl::TimeDuration::Millisecond>;

    private:
        constexpr static u32 SPRITE_SIZE{ 4 };

        // #define RGBA(r, g, b, a) r, g, b, a

        constexpr static inline u8 pixel_array[] = {
            0xff, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0x80,
            0xff, 0x00, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
            0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x80, 0xff, 0x80, 0x00, 0xff, 0xff,
            0x00, 0x00, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff
        };
        // static inline constexpr void* pixel_array = ((void*)(pixels.data()->data()));

    public:
        application()
        {
            bool ret = this->init_subsystem(Subsystem::Video);
            runtime_assert(ret, "failed to init SDL subsystem");

            // ret = m_renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
            // runtime_assert(ret, "failed to set_blend_mode on target2");
            //
            // ret = m_sprite.update(pixel_array, 4 * 4);
            // runtime_assert(ret, "failed to set_blend_mode on target2");
            //
            // ret = m_sprite.set_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
            // runtime_assert(ret, "failed to set_blend_mode on target2");
            //
            // ret = m_target1.set_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
            // runtime_assert(ret, "failed to set_blend_mode on target2");
            //
            // ret = m_target2.set_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
            // runtime_assert(ret, "failed to set_blend_mode on target2");
        }

        ~application()
        {
            SDL3::SDL_Quit();
        }

        // bool loop()
        //{
        //     SDL3::SDL_Event event;
        //     while (SDL3::SDL_PollEvent(&event))
        //         if (event.type == SDL3::SDL_EVENT_QUIT ||
        //             (event.type == SDL3::SDL_EVENT_KEY_DOWN &&
        //              (event.key.keysym.sym == SDL3::SDLK_ESCAPE ||
        //               event.key.keysym.sym == SDL3::SDLK_q)))
        //             return 0;

        //    // Note we fill with transparent color, not black
        //    m_renderer.set_draw_color({ 0, 0, 0, 0 });

        //    // Fill base texture with sprite texture
        //    m_renderer.set_target(m_target1);
        //    m_renderer.clear();
        //    m_renderer.copy(m_sprite);

        //    // Repeat several cycles of flip-flop tiling
        //    for (int i = 0; i < 4; i++)
        //    {
        //        m_renderer.set_target(m_target2);
        //        m_renderer.clear();
        //        m_renderer.copy(m_target1, ds::rect<i32>::null(),
        //                        ds::rect<i32>{ 0, 0, 512 / 2, 512 / 2 },
        //                        cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);
        //        m_renderer.copy(m_target1, ds::rect<i32>::null(),
        //                        ds::rect<i32>{ 512 / 2, 0, 512 / 2, 512 / 2 },
        //                        cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);
        //        m_renderer.copy(m_target1, ds::rect<i32>::null(),
        //                        ds::rect<i32>{ 0, 512 / 2, 512 / 2, 512 / 2 },
        //                        cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);
        //        m_renderer.copy(m_target1, ds::rect<i32>::null(),
        //                        ds::rect<i32>{ 512 / 2, 512 / 2, 512 / 2, 512 / 2 },
        //                        cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

        //        // Swap textures to copy recursively
        //        std::swap(m_target1, m_target2);
        //    }

        //    // Draw result to screen
        //    m_renderer.set_target();
        //    m_renderer.clear();
        //    // 640, 480
        //    m_renderer.copy(m_target1, ds::rect<i32>::null(),
        //                    ds::rect<i32>{ (640 - 480) / 2, 0, 480, 480 },
        //                    cast::to<f64>(SDL3::SDL_GetTicks()) / 10000.0 * 360.0);

        //    m_renderer.present();

        //    // Frame limiter
        //    SDL3::SDL_Delay(1);
        //    return true;
        //}

        // void loop2()
        //{
        //     PixelInspector pixels(320, 240, 4);

        //    {
        //        // Clear, draw color
        //        m_renderer.set_draw_color({ 1, 2, 3 });

        //        sdl::color&& c = m_renderer.get_draw_color();
        //        runtime_assert(c.r == 1 && c.g == 2 && c.b == 3 && c.a == 255, "test failed");

        //        m_renderer.clear();
        //        pixels.Retrieve(m_renderer);

        //        auto res = pixels.Test(0, 0, 1, 2, 3);
        //        runtime_assert(res, "test failed");

        //        m_renderer.present();
        //        SDL3::SDL_Delay(1000);
        //    }

        //    {
        //        // Draw points
        //        m_renderer.set_draw_color(sdl::color{ 0, 0, 0 });
        //        m_renderer.clear();

        //        m_renderer.set_draw_color(sdl::color{ 255, 128, 0 });
        //        m_renderer.draw_point({ 10, 10 });

        //        m_renderer.set_draw_color(sdl::color{ 0, 255, 128 });
        //        m_renderer.draw_point(ds::point<i32>{ 20, 20 });

        //        m_renderer.set_draw_color(sdl::color{ 128, 0, 255 });
        //        std::vector<ds::point<f32>> points = { { 30, 30 } };
        //        m_renderer.draw_points(points);
        //        pixels.Retrieve(m_renderer);

        //        auto res1 = pixels.Test3x3(10, 10, 0x020, 255, 128, 0);
        //        auto res2 = pixels.Test3x3(20, 20, 0x020, 0, 255, 128);
        //        auto res3 = pixels.Test3x3(30, 30, 0x020, 128, 0, 255);
        //        runtime_assert(res1, "test failed");
        //        runtime_assert(res2, "test failed");
        //        runtime_assert(res3, "test failed");

        //        m_renderer.present();
        //        SDL3::SDL_Delay(1000);
        //    }

        //    {
        //        // Draw lines
        //        m_renderer.set_draw_color(sdl::color{ 0, 0, 0 });
        //        m_renderer.clear();

        //        m_renderer.set_draw_color(sdl::color{ 255, 128, 0 });
        //        m_renderer.draw_line({ 10, 10 }, { 10, 50 });

        //        m_renderer.set_draw_color(sdl::color{ 0, 255, 128 });
        //        m_renderer.draw_line(ds::point<i32>{ 20, 10 }, ds::point<i32>{ 20, 50 });

        //        m_renderer.set_draw_color(sdl::color{ 128, 0, 255 });
        //        std::vector<ds::point<f32>> points = { { 30, 10 }, { 30, 50 } };
        //        m_renderer.draw_lines(points);

        //        pixels.Retrieve(m_renderer);

        //        auto res1 = pixels.Test3x3(10, 20, 0x222, 255, 128, 0);
        //        auto res2 = pixels.Test3x3(20, 20, 0x222, 0, 255, 128);
        //        auto res3 = pixels.Test3x3(30, 20, 0x222, 128, 0, 255);
        //        runtime_assert(res1, "test failed");
        //        runtime_assert(res2, "test failed");
        //        runtime_assert(res3, "test failed");

        //        m_renderer.present();
        //        SDL3::SDL_Delay(1000);
        //    }
        //}

        bool is_initialized() const
        {
            return m_initialized.load(std::memory_order_relaxed);
        }

        bool init_subsystem(Subsystem flags)
        {
            bool init{ m_initialized.load(std::memory_order_relaxed) };
            i32 result = SDL3::SDL_Init(flags);
            runtime_assert(result == 0, "failed to init subsystem");
            m_initialized.store(!init && 0 == SDL3::SDL_Init(flags), std::memory_order_relaxed);
            return m_initialized.load(std::memory_order_relaxed);
        }

        sdl::window& window()
        {
            return m_window;
        }

        sdl::renderer& renderer()
        {
            return m_renderer;
        }

        timer_t& timer()
        {
            return m_timer;
        }

    private:
        application(const sdl::application& other) = delete;
        application(sdl::application&& other) = delete;

        application& operator=(const application& other) = delete;
        application& operator=(application&& other) = delete;

        void report_error()
        {
            // SDL3::SDL_GetErrBuf();
        }

    private:
        // TODO: implement single instance enforcement
        std::once_flag init_flag{};
        // microsecond resolution
        timer_t m_timer{};

        sdl::window m_window{ "Roguelite" };
        // #ifndef ROGUELIKE_TESTS_ENABLED
        sdl::renderer m_renderer{ m_window, renderer::driver::DirectX12 };
        sdl::texture m_sprite{ m_renderer, SDL3::SDL_PIXELFORMAT_ARGB8888,
                               SDL3::SDL_TEXTUREACCESS_STATIC, 4, 4 };
        sdl::texture m_target1{ m_renderer, SDL3::SDL_PIXELFORMAT_ARGB8888,
                                SDL3::SDL_TEXTUREACCESS_TARGET, 512, 512 };
        sdl::texture m_target2{ m_renderer, SDL3::SDL_PIXELFORMAT_ARGB8888,
                                SDL3::SDL_TEXTUREACCESS_TARGET, 512, 512 };
        // #endif
        static inline std::atomic<bool> m_initialized{ false };
    };
}
