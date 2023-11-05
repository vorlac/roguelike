#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "core/numeric_types.hpp"
#include "sdl/renderer.hpp"
#include "sdl/window.hpp"

namespace SDL3
{
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_init.h>
}

namespace rl::sdl
{
    class sdl_app
    {
    public:
        enum Subsystem : u16_fast {
            Timer    = SDL3::SDL_INIT_TIMER,
            Audio    = SDL3::SDL_INIT_AUDIO,
            Video    = SDL3::SDL_INIT_VIDEO,
            Joystick = SDL3::SDL_INIT_JOYSTICK,
            Haptic   = SDL3::SDL_INIT_HAPTIC,
            Gamepad  = SDL3::SDL_INIT_GAMEPAD,
            Events   = SDL3::SDL_INIT_EVENTS,
            Sensor   = SDL3::SDL_INIT_SENSOR,

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

    private:
        static constexpr u32 SPRITE_SIZE{ 4 };

        // #define RGBA(r, g, b, a) r, g, b, a

        static constexpr inline std::array<std::array<u8, 4>, 4 * 4> pixels = {
            sdl::color(0xff, 0x00, 0x00, 0xff), sdl::color(0xff, 0x80, 0x00, 0xff),
            sdl::color(0xff, 0xff, 0x00, 0xff), sdl::color(0x80, 0xff, 0x00, 0xff),
            sdl::color(0xff, 0x00, 0x80, 0xff), sdl::color(0xff, 0xff, 0xff, 0xff),
            sdl::color(0x00, 0x00, 0x00, 0x00), sdl::color(0x00, 0xff, 0x00, 0xff),
            sdl::color(0xff, 0x00, 0xff, 0xff), sdl::color(0x00, 0x00, 0x00, 0x00),
            sdl::color(0x00, 0x00, 0x00, 0xff), sdl::color(0x00, 0xff, 0x80, 0xff),
            sdl::color(0x80, 0x00, 0xff, 0xff), sdl::color(0x00, 0x00, 0xff, 0xff),
            sdl::color(0x00, 0x80, 0xff, 0xff), sdl::color(0x00, 0xff, 0xff, 0xff),
        };
        static inline constexpr void* pixel_array = ((void*)(pixels.data()->data()));

    public:
        sdl_app()
        {
            this->init_subsystem(Subsystem::All);
            m_renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_BLEND);

            m_sprite.update(pixel_array, 4 * 4);
            m_sprite.set_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
            m_target1.set_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
            m_target2.set_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
        }

        ~sdl_app()
        {
            SDL3::SDL_Quit();
        }

        bool loop()
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

    private:
        sdl_app(const sdl::sdl_app& other) = delete;
        sdl_app(sdl::sdl_app&& other)      = delete;

        sdl_app& operator=(const sdl_app& other) = delete;
        sdl_app& operator=(sdl_app&& other)      = delete;

        void report_error()
        {
            // SDL3::SDL_GetErrBuf();
        }

    private:
        sdl::window m_window{};
        sdl::renderer m_renderer{ m_window, SDL3::SDL_RENDERER_ACCELERATED };
        sdl::texture m_sprite{ m_renderer, SDL3::SDL_PIXELFORMAT_ARGB8888,
                               SDL3::SDL_TEXTUREACCESS_STATIC, 4, 4 };
        sdl::texture m_target1{ m_renderer, SDL3::SDL_PIXELFORMAT_ARGB8888,
                                SDL3::SDL_TEXTUREACCESS_TARGET, 512, 512 };
        sdl::texture m_target2{ m_renderer, SDL3::SDL_PIXELFORMAT_ARGB8888,
                                SDL3::SDL_TEXTUREACCESS_TARGET, 512, 512 };

        std::once_flag init_flag{};
        static inline std::atomic<bool> m_initialized{ false };
    };
}
