
#include <ctime>
#include <memory>
#include <utility>
#include <vector>

#include <stdlib.h>
#include <fmt/format.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/numeric_types.hpp"
#include "sdl/color.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer.hpp"
#include "sdl/surface.hpp"
#include "sdl/tests/data/icon.hpp"
#include "sdl/texture.hpp"
#include "sdl/window.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

namespace rl::sdl::test {
    static sdl::texture create_texture(std::shared_ptr<sdl::renderer> renderer,
                                       std::vector<u8>& data, ds::dimensions<i32>& size)
    {
        SDL3::SDL_RWops* src = SDL3::SDL_RWFromConstMem(data.data(), data.size());
        if (src != nullptr)
        {
            sdl::surface surface = SDL3::SDL_LoadBMP_RW(src, SDL_TRUE);
            if (surface.is_valid())
            {
                /* Treat white as transparent */
                sdl::color c{ 255, 255, 255 };

                surface.set_color_key(true, c.rgb(surface.get_format_full()));
                sdl::texture texture{ renderer, surface };

                auto dims = surface.size();
                ds::dimensions<i32> dims2 = {
                    surface.sdl_handle()->w,
                    surface.sdl_handle()->h,
                };

                runtime_assert(dims == dims2, "??");

                size.width = surface.sdl_handle()->w;
                size.height = surface.sdl_handle()->h;

                return texture;
            }
        }

        runtime_assert(false, "failed to create texture");
        return sdl::texture{ nullptr };
    }

    static void move_sprites(sdl::window& window, sdl::texture& sprite, auto&& sprites,
                             auto& sprite_size)
    {
        ds::dimensions<i32> window_size = window.get_render_size();

        auto renderer = window.renderer();
        /* Draw a gray background */
        renderer->set_draw_color({ 0xA0, 0xA0, 0xA0, 0xFF });
        renderer->clear();

        /* Move the sprite, bounce at the wall, and draw */
        for (auto&& sprite_info : sprites)
        {
            auto&& [velocity, position] = sprite_info;

            position.pt.x += velocity.pt.x;
            if ((position.pt.x < 0) ||
                (position.pt.x >= cast::to<f32>(window_size.width - sprite_size.width)))
            {
                velocity.pt.x = -velocity.pt.x;
                position.pt.x += velocity.pt.x;
            }

            position.pt.y += velocity.pt.y;
            if ((position.pt.y < 0) ||
                (position.pt.y >= cast::to<f32>(window_size.height - sprite_size.height)))
            {
                velocity.pt.y = -velocity.pt.y;
                position.pt.y += velocity.pt.y;
            }

            /* Blit the sprite onto the screen */
            renderer->draw_texture(sprite, ds::rect<f32>::null(), position);
        }
        /* Update the screen! */
        renderer->present();
    }

    int execute_sprite_drawing_tests(sdl::window& window)
    {
        i32 return_code = -1;

        ds::dimensions<i32> window_size = window.get_render_size();
        ds::dimensions<i32> sprite_size{ 0, 0 };

        std::vector<u8> icon_data = { icon_bmp, icon_bmp + icon_bmp_len };

        sdl::texture sprite = create_texture(window.renderer(), icon_data, sprite_size);
        if (!sprite.is_valid())
            return false;

        /* Initialize the sprite positions */
        constexpr static size_t SPRITE_COUNT = 100;
        constexpr static size_t MAX_SPEED = 1;
        std::vector<std::pair<ds::rect<f32>, ds::rect<f32>>> sprites(SPRITE_COUNT);

        srand((u32)time(nullptr));
        for (auto& sprite_info : sprites)
        {
            auto& [velocity, position] = sprite_info;

            position.pt.x = rl::cast::to<f32>(rand() % (window_size.width - sprite_size.width));
            position.pt.y = rl::cast::to<f32>(rand() % (window_size.height - sprite_size.height));
            position.size.width = rl::cast::to<f32>(sprite_size.width);
            position.size.height = rl::cast::to<f32>(sprite_size.height);

            velocity.pt.x = 0.0f;
            velocity.pt.y = 0.0f;
            while (velocity.pt.is_zero())
            {
                velocity.pt.x = cast::to<f32>((rand() % (MAX_SPEED * 2 + 1)) - MAX_SPEED);
                velocity.pt.y = cast::to<f32>((rand() % (MAX_SPEED * 2 + 1)) - MAX_SPEED);
            }
        }

        bool done = false;
        while (!done)
        {
            SDL3::SDL_Event e;
            while (SDL3::SDL_PollEvent(&e))
                if (e.type == SDL3::SDL_EVENT_QUIT || e.type == SDL3::SDL_EVENT_KEY_DOWN)
                    done = true;

            move_sprites(window, sprite, sprites, sprite_size);

            if (done)
                break;
        }

        return return_code;
    }
}
