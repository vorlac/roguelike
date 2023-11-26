#include <fmt/format.h>

#include "sdl/color.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer.hpp"
#include "sdl/surface.hpp"
#include "sdl/tests/data/images.hpp"
#include "sdl/texture.hpp"
#include "utils/conversions.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_test.h>
SDL_C_LIB_END

namespace rl::sdl::test {
    constexpr rl::i32 TESTRENDER_SCREEN_W{ 1024 };
    constexpr rl::i32 TESTRENDER_SCREEN_H{ 768 };
    constexpr SDL3::SDL_PixelFormatEnum RENDER_COMPARE_FORMAT{ SDL3::SDL_PIXELFORMAT_ARGB8888 };
    constexpr rl::u32 RENDER_COLOR_CLEAR{ 0xFF000000 };
    constexpr rl::u32 RENDER_COLOR_GREEN{ 0xFF00FF00 };
    constexpr rl::i32 ALLOWABLE_ERROR_OPAQUE{ 0 };
    constexpr rl::i32 ALLOWABLE_ERROR_BLENDED{ 64 };

    static bool has_draw_color(sdl::Renderer& renderer)
    {
        /* Set color. */
        rl::sdl::Color set_clr{ 100, 100, 100, 100 };
        if (!renderer.set_draw_color(set_clr))
            return false;

        rl::sdl::Color get_clr = renderer.get_draw_color();
        if (!renderer.set_draw_color({ 0, 0, 0 }))
            return false;

        if (set_clr != get_clr)
            return false;

        return true;
    }

    static bool has_blend_modes(sdl::Renderer& renderer)
    {
        SDL3::SDL_BlendMode set_mode = SDL3::SDL_BLENDMODE_NONE;
        if (!renderer.set_draw_blend_mode(set_mode))
            return false;
        SDL3::SDL_BlendMode get_mode = renderer.get_draw_blend_mode();
        if (set_mode != get_mode)
            return false;

        set_mode = SDL3::SDL_BLENDMODE_ADD;
        if (!renderer.set_draw_blend_mode(set_mode))
            return false;
        get_mode = renderer.get_draw_blend_mode();
        if (set_mode != get_mode)
            return false;

        set_mode = SDL3::SDL_BLENDMODE_MOD;
        if (!renderer.set_draw_blend_mode(set_mode))
            return false;
        get_mode = renderer.get_draw_blend_mode();
        if (set_mode != get_mode)
            return false;

        set_mode = SDL3::SDL_BLENDMODE_NONE;
        if (!renderer.set_draw_blend_mode(set_mode))
            return false;
        get_mode = renderer.get_draw_blend_mode();
        if (set_mode != get_mode)
            return false;

        return true;
    }

    static rl::sdl::Texture load_test_face(sdl::Renderer& renderer)
    {
        rl::sdl::Surface face = sdl::test::image::ImageFace();
        if (!face.is_valid())
            return nullptr;

        rl::sdl::Texture tface{ renderer, face };
        if (!tface.is_valid())
            sdl_assert(tface.is_valid(), "failed to create texture");

        return tface;
    }

    static bool has_tex_color(sdl::Renderer& renderer)
    {
        rl::sdl::Texture tface = load_test_face(renderer);
        if (tface.is_valid())
        {
            sdl::Color set_clr{ 100, 100, 100 };
            if (!tface.set_color_mod(set_clr))
                return false;

            sdl::Color get_clr{ tface.get_color_mod() };
            if (get_clr != set_clr)
                return false;
        }

        return true;
    }

    static bool has_tex_alpha(sdl::Renderer& renderer)
    {
        sdl::Texture tface = load_test_face(renderer);
        if (!tface.is_valid())
            return false;
        if (!tface.set_alpha_mod(100))
            return false;
        if (rl::u8 alpha = tface.get_alpha_mod(); alpha != 100)
            return false;

        return true;
    }

    /**
     * @brief compares renderer's surface with a reference surface
     * */
    static void compare(sdl::Renderer& renderer, sdl::Surface& reference_surface,
                        int allowable_error)
    {
        rl::u8* pixels{ nullptr };

        // Read pixels.
        pixels = static_cast<rl::u8*>(
            SDL3::SDL_malloc(4 * sdl::test::TESTRENDER_SCREEN_W * sdl::test::TESTRENDER_SCREEN_H));
        if (pixels == nullptr)
            return;

        // Explicitly specify the rect in case the window isn't the expected size
        ds::rect<i32> rect{
            0,
            0,
            sdl::test::TESTRENDER_SCREEN_W,
            sdl::test::TESTRENDER_SCREEN_H,
        };
        rl::i32 pitch = sdl::test::TESTRENDER_SCREEN_W * 4;
        renderer.read_pixels(rect, sdl::test::RENDER_COMPARE_FORMAT, pixels, pitch);

        /* Create surface. */
        sdl::Surface test_surface{ pixels, rect.size.width, rect.size.height, pitch,
                                   sdl::test::RENDER_COMPARE_FORMAT };

        /* Compare surface. */
        rl::i32 failures = test_surface.compare(reference_surface, allowable_error);
    }

    /**
     * @brief Clears the screen.
     * */
    static int clear_screen(sdl::Renderer& renderer)
    {
        /* Make current */
        renderer.present();
        rl::i32 ret = renderer.set_draw_color({ 0, 0, 0 });
        ret = renderer.clear();

        /* Set defaults. */
        ret = renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_NONE);
        ret = renderer.set_draw_color({ 255, 255, 255 });

        return 0;
    }

    /**
     * @brief Tests call to SDL3::SDL_GetNumRenderDrivers
     * */
    static int render_test_get_num_render_drivers(sdl::Renderer& renderer)
    {
        auto drivers = renderer.get_render_drivers();
        sdl_assert(drivers.size() > 0, "no render drivers found");
        return TEST_COMPLETED;
    }

    /**
     * @brief Tests the SDL primitives for rendering.
     * */
    static int render_test_primitives(sdl::Renderer& renderer)
    {
        bool ret = true;

        /* Clear surface. */
        clear_screen(renderer);

        /* Need drawcolor or just skip test. */
        has_draw_color(renderer);

        /* Draw a rectangle. */
        ds::rect<rl::f32> rect{
            40.0f,
            0.0f,
            40.0f,
            80.0f,
        };
        ret = renderer.set_draw_color({ 13, 73, 200 });
        ret = renderer.fill_rect(rect);

        /* Draw a rectangle. */
        rect = {
            10.0f,
            10.0f,
            60.0f,
            40.0f,
        };
        ret = renderer.set_draw_color({ 200, 0, 100 });
        ret = renderer.fill_rect(rect);

        /* Draw some points like so:
         * X.X.X.X..
         * .X.X.X.X.
         * X.X.X.X.. */
        i32 check_fail_count_1 = 0;
        i32 check_fail_count_2 = 0;
        for (rl::i32 y = 0; y < 3; y++)
        {
            for (rl::i32 x = y % 2; x < sdl::test::TESTRENDER_SCREEN_W; x += 2)
            {
                ret = renderer.set_draw_color({
                    static_cast<u8>(x * y),
                    static_cast<u8>(x * y / 2),
                    static_cast<u8>(x * y / 3),
                });
                if (!ret)
                    ++check_fail_count_1;

                ret = renderer.draw_point({ cast::to<f32>(x), cast::to<f32>(y) });
                if (!ret)
                    check_fail_count_2++;
            }
        }

        sdl_assert(check_fail_count_1 == 0, "render test 1 failed");
        sdl_assert(check_fail_count_2 == 0, "render test 2 failed");

        // draw stuff...
        renderer.set_draw_color({ 0, 255, 0 });
        renderer.draw_line({ 0.0f, 30.0f },
                           { cast::to<float>(sdl::test::TESTRENDER_SCREEN_W), 30.0f });

        renderer.set_draw_color({ 55, 55, 5 });
        renderer.draw_line({ 40.0f, 30.0f }, { 40.0f, 60.0f });

        renderer.set_draw_color({ 5, 105, 105 });
        renderer.draw_line({ 0.0f, 0.0f }, { 29.0f, 29.0f });
        renderer.draw_line({ 29.0f, 30.0f }, { 0.0f, 59.0f });
        renderer.draw_line({ 79.0f, 0.0f }, { 50.0f, 29.0f });
        renderer.draw_line({ 79.0f, 59.0f }, { 50.0f, 30.0f });

        // compare to reference image to see if it's the same
        sdl::Surface reference_surface = sdl::test::image::ImagePrimitives();
        compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_OPAQUE);

        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * @brief tests the SDL primitives with alpha for rendering
     * */
    static int render_test_primitives_blend(sdl::Renderer& renderer)
    {
        // Clear surface.
        clear_screen(renderer);

        // Need drawcolor and blendmode or just skip test.
        has_draw_color(renderer);
        has_blend_modes(renderer);

        // Create some rectangles for each blend mode.
        renderer.set_draw_color({ 255, 255, 255, 0 });
        renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_NONE);
        renderer.fill_rect(ds::rect<i32>::null());

        ds::rect<f32> rect{
            { 10.0f, 25.0f },
            { 40.0f, 25.0f },
        };

        renderer.set_draw_color({ 240, 10, 10, 75 });
        renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_ADD);
        renderer.fill_rect(rect);

        rect = {
            { 30.0f, 40.0f },
            { 45.0f, 15.0f },
        };

        renderer.set_draw_color({ 10, 240, 10, 100 });
        renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_BLEND);
        renderer.fill_rect(rect);

        rect = {
            { 25.0f, 25.0f },
            { 25.0f, 25.0f },
        };

        renderer.set_draw_color({ 10, 10, 240, 125 });
        renderer.set_draw_blend_mode(SDL3::SDL_BLENDMODE_NONE);
        renderer.fill_rect(rect);

        bool ret = true;

        // Draw blended lines
        rl::i32 check_fail_count_1{ 0 };
        rl::i32 check_fail_count_2{ 0 };
        rl::i32 check_fail_count_3{ 0 };
        for (i32 i = 0; i < sdl::test::TESTRENDER_SCREEN_W; i += 2)
        {
            auto blend_mode = (((i / 2) % 3) == 0) ? SDL3::SDL_BLENDMODE_BLEND
                            : (((i / 2) % 3) == 1) ? SDL3::SDL_BLENDMODE_ADD
                                                   : SDL3::SDL_BLENDMODE_NONE;

            if (!renderer.set_draw_color(
                    { abs((60 + 2 * i) % 255), abs((240 - 2 * i) % 255), abs((3 * i) % 255) }))
                ++check_fail_count_1;
            if (!renderer.set_draw_blend_mode(blend_mode))
                check_fail_count_2++;
            if (!renderer.draw_line({ 0.0f, 0.0f }, { (float)i, 59.0f }))
                check_fail_count_3++;
        }

        sdl_assert(check_fail_count_1 == 0, "render test 1 failed");
        sdl_assert(check_fail_count_2 == 0, "render test 2 failed");
        sdl_assert(check_fail_count_3 == 0, "render test 3 failed");

        check_fail_count_1 = 0;
        check_fail_count_2 = 0;
        check_fail_count_3 = 0;
        for (rl::i32 i = 0; i < sdl::test::TESTRENDER_SCREEN_H; i += 2)
        {
            auto blend_mode = (((i / 2) % 3) == 0) ? SDL3::SDL_BLENDMODE_BLEND
                            : (((i / 2) % 3) == 1) ? SDL3::SDL_BLENDMODE_ADD
                                                   : SDL3::SDL_BLENDMODE_NONE;
            if (!renderer.set_draw_color(
                    { abs((60 + 2 * i) % 255), abs((240 - 2 * i) % 255), abs((3 * i) % 255) }))
                ++check_fail_count_1;

            if (!renderer.set_draw_blend_mode(blend_mode))
                check_fail_count_2++;

            if (!renderer.draw_line({ 0.0f, 0.0f }, { 79.0f, (float)i }))
                check_fail_count_3++;
        }

        sdl_assert(check_fail_count_1 == 0, "render test 1 failed");
        sdl_assert(check_fail_count_2 == 0, "render test 2 failed");
        sdl_assert(check_fail_count_3 == 0, "render test 3 failed");

        /* Draw points. */
        check_fail_count_1 = 0;
        check_fail_count_2 = 0;
        check_fail_count_3 = 0;
        for (i32 j = 0; j < sdl::test::TESTRENDER_SCREEN_H; j += 3)
        {
            for (i32 i = 0; i < sdl::test::TESTRENDER_SCREEN_W; i += 3)
            {
                auto blend_mode = ((((i + j) / 3) % 3) == 0) ? SDL3::SDL_BLENDMODE_BLEND
                                : ((((i + j) / 3) % 3) == 1) ? SDL3::SDL_BLENDMODE_ADD
                                                             : SDL3::SDL_BLENDMODE_NONE;

                if (!renderer.set_draw_color({ abs((j * 4) % 255), abs((i * 3) % 255),
                                               abs((j * 4) % 255), abs((i * 3) % 255) }))
                    ++check_fail_count_1;
                if (!renderer.set_draw_blend_mode(blend_mode))
                    check_fail_count_2++;
                if (!renderer.draw_point({ (float)i, (float)j }))
                    check_fail_count_3++;
            }
        }

        sdl_assert(check_fail_count_1 == 0, "render test 1 failed");
        sdl_assert(check_fail_count_2 == 0, "render test 2 failed");
        sdl_assert(check_fail_count_3 == 0, "render test 3 failed");

        /* See if it's the same. */
        sdl::Surface reference_surface = sdl::test::image::ImagePrimitivesBlend();
        compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_BLENDED);

        /* Make current */
        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * @brief tests some blitting routines.
     * */
    static int render_test_blit(sdl::Renderer& renderer)
    {
        i32 ret = 0;

        /* Clear surface. */
        clear_screen(renderer);

        /* Need drawcolor or just skip test. */
        has_draw_color(renderer);

        /* Create face surface. */
        sdl::Texture tface = load_test_face(renderer);
        if (!tface.is_valid())
            return TEST_ABORTED;

        ds::dims<i32> tdims{ 0, 0 };
        SDL3::SDL_TextureAccess taccess = SDL3::SDL_TextureAccess(0);
        SDL3::SDL_PixelFormatEnum tformat = SDL3::SDL_PixelFormatEnum(0);
        tface.query_texture(tformat, taccess, tdims);
        ds::rect<f32> rect{
            ds::point<f32>{ 0, 0 },
            ds::dims<f32>{
                cast::to<float>(tdims.width),
                cast::to<float>(tdims.height),
            },
        };

        rl::i32 ni = sdl::test::TESTRENDER_SCREEN_W - tdims.width;
        rl::i32 nj = sdl::test::TESTRENDER_SCREEN_H - tdims.height;

        /* Loop blit. */
        rl::i32 check_fail_count_1 = 0;
        for (rl::i32 j = 0; j <= nj; j += 4)
        {
            for (rl::i32 i = 0; i <= ni; i += 4)
            {
                /* Blitting. */
                rect.pt.x = (float)i;
                rect.pt.y = (float)j;

                if (!renderer.draw_texture(tface, ds::rect<rl::f32>::null(), rect))
                    ++check_fail_count_1;
            }
        }

        /* See if it's the same */
        sdl::Surface reference_surface = sdl::test::image::ImageBlit();
        compare(renderer, reference_surface, ALLOWABLE_ERROR_OPAQUE);

        /* Make current */
        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * @brief Blits doing color tests.
     * */
    static int render_test_blit_color(sdl::Renderer& renderer)
    {
        int ret = 0;

        /* Clear surface. */
        clear_screen(renderer);

        /* Create face surface. */
        sdl::Texture tface = load_test_face(renderer);
        if (!tface.is_valid())
            return TEST_ABORTED;

        /* Constant values. */
        ds::dims<i32> tdims{ 0, 0 };
        SDL3::SDL_TextureAccess taccess = SDL3::SDL_TextureAccess(0);
        SDL3::SDL_PixelFormatEnum tformat = SDL3::SDL_PixelFormatEnum(0);
        tface.query_texture(tformat, taccess, tdims);
        ds::rect<f32> rect{
            { 0, 0 },
            {
                cast::to<float>(tdims.width),
                cast::to<float>(tdims.height),
            },
        };

        rl::i32 ni = sdl::test::TESTRENDER_SCREEN_W - tdims.width;
        rl::i32 nj = sdl::test::TESTRENDER_SCREEN_H - tdims.height;

        /* Test blitting with color mod. */
        rl::i32 check_fail_count_1 = 0;
        rl::i32 check_fail_count_2 = 0;
        for (rl::i32 j = 0; j <= nj; j += 4)
        {
            for (rl::i32 i = 0; i <= ni; i += 4)
            {
                /* Set color mod. */
                sdl::Color color_mod{ (255 / nj) * j, (255 / ni) * i, (255 / nj) * j };
                if (!tface.set_color_mod(color_mod))
                    ++check_fail_count_1;

                /* Blitting. */
                rect.pt.x = (float)i;
                rect.pt.y = (float)j;

                if (!renderer.draw_texture(tface, ds::rect<f32>::null(), rect))
                    check_fail_count_2++;
            }
        }

        /* See if it's the same. */
        sdl::Surface reference_surface = sdl::test::image::ImageBlitColor();
        compare(renderer, reference_surface, ALLOWABLE_ERROR_OPAQUE);

        /* Make current */
        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * @brief tests blitting with alpha.
     * */
    static int render_test_blit_alpha(sdl::Renderer& renderer)
    {
        int ret = 0;

        /* Clear surface. */
        clear_screen(renderer);

        has_tex_alpha(renderer);
        /* Need alpha or just skip test. */
        if (!has_tex_alpha(renderer))
            return -1;

        /* Create face surface. */
        sdl::Texture tface = load_test_face(renderer);
        if (!tface.is_valid())
            return TEST_ABORTED;

        /* Constant values. */
        ds::dims<i32> tdims{ 0, 0 };
        SDL3::SDL_TextureAccess taccess = SDL3::SDL_TextureAccess(0);
        SDL3::SDL_PixelFormatEnum tformat = SDL3::SDL_PixelFormatEnum(0);
        tface.query_texture(tformat, taccess, tdims);
        ds::rect<f32> rect{
            { 0, 0 },
            {
                cast::to<float>(tdims.width),
                cast::to<float>(tdims.height),
            },
        };

        rl::i32 ni = sdl::test::TESTRENDER_SCREEN_W - tdims.width;
        rl::i32 nj = sdl::test::TESTRENDER_SCREEN_H - tdims.height;

        /* Test blitting with alpha mod. */
        rl::i32 check_fail_count_1 = 0;
        rl::i32 check_fail_count_2 = 0;
        for (rl::i32 j = 0; j <= nj; j += 4)
        {
            for (rl::i32 i = 0; i <= ni; i += 4)
            {
                /* Set alpha mod. */
                if (!tface.set_alpha_mod(cast::to<u8>((255 / ni) * i)))
                    ++check_fail_count_1;

                /* Blitting. */
                rect.pt.x = (float)i;
                rect.pt.y = (float)j;
                if (!renderer.draw_texture(tface, ds::rect<f32>::null(), rect))
                    check_fail_count_2++;
            }
        }

        /* See if it's the same. */
        sdl::Surface reference_surface = sdl::test::image::ImageBlitAlpha();
        compare(renderer, reference_surface, ALLOWABLE_ERROR_BLENDED);

        /* Make current */
        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * @brief tests a blend mode.
     * */
    static void test_blit_blend_mode(sdl::Renderer& renderer, sdl::Texture& tface, int mode)
    {
        /* Clear surface. */
        clear_screen(renderer);

        /* Constant values. */
        ds::dims<i32> tdims{ 0, 0 };
        SDL3::SDL_TextureAccess taccess = SDL3::SDL_TextureAccess(0);
        SDL3::SDL_PixelFormatEnum tformat = SDL3::SDL_PixelFormatEnum(0);
        tface.query_texture(tformat, taccess, tdims);
        ds::rect<f32> rect{
            { 0, 0 },
            {
                cast::to<float>(tdims.width),
                cast::to<float>(tdims.height),
            },
        };

        rl::i32 ni = sdl::test::TESTRENDER_SCREEN_W - tdims.width;
        rl::i32 nj = sdl::test::TESTRENDER_SCREEN_H - tdims.height;

        /* Test blend mode. */
        rl::i32 check_fail_count_1 = 0;
        rl::i32 check_fail_count_2 = 0;
        for (rl::i32 j = 0; j <= nj; j += 4)
        {
            for (rl::i32 i = 0; i <= ni; i += 4)
            {
                /* Set blend mode. */
                // ret = SDL3::SDL_SetTextureBlendMode(tface, (SDL3::SDL_BlendMode)mode);
                if (!tface.set_blend_mode(static_cast<SDL3::SDL_BlendMode>(mode)))
                    ++check_fail_count_1;

                /* Blitting. */
                rect.pt.x = (float)i;
                rect.pt.y = (float)j;

                if (!renderer.draw_texture(tface, ds::rect<f32>::null(), rect))
                    check_fail_count_2++;
            }
        }
    }

    /**
     * @brief _________
     * */
    static int render_test_blit_blend(sdl::Renderer& renderer)
    {
        int ret = 0;

        has_blend_modes(renderer);
        has_tex_color(renderer);
        has_tex_alpha(renderer);

        {
            /* Create face surface. */
            sdl::Texture tface = load_test_face(renderer);
            if (!tface.is_valid())
                return TEST_ABORTED;

            ds::dims<i32> tdims{ 0, 0 };
            SDL3::SDL_TextureAccess taccess = SDL3::SDL_TextureAccess(0);
            SDL3::SDL_PixelFormatEnum tformat = SDL3::SDL_PixelFormatEnum(0);
            tface.query_texture(tformat, taccess, tdims);
            ds::rect<f32> rect{
                { 0, 0 },
                {
                    cast::to<float>(tdims.width),
                    cast::to<float>(tdims.height),
                },
            };

            rl::i32 ni = sdl::test::TESTRENDER_SCREEN_W - tdims.width;
            rl::i32 nj = sdl::test::TESTRENDER_SCREEN_H - tdims.height;

            tface.set_alpha_mod(100);

            {
                {
                    /* Test None. */
                    test_blit_blend_mode(renderer, tface, SDL3::SDL_BLENDMODE_NONE);
                    sdl::Surface reference_surface = sdl::test::image::ImageBlitBlendNone();
                    /* Compare, then Present */
                    compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_OPAQUE);
                    renderer.present();
                }

                {
                    /* Test Blend. */
                    test_blit_blend_mode(renderer, tface, SDL3::SDL_BLENDMODE_BLEND);
                    sdl::Surface reference_surface = sdl::test::image::ImageBlitBlend();
                    /* Compare, then Present */
                    compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_BLENDED);
                    renderer.present();
                }

                {
                    /* Test Add. */
                    test_blit_blend_mode(renderer, tface, SDL3::SDL_BLENDMODE_ADD);
                    sdl::Surface reference_surface = sdl::test::image::ImageBlitBlendAdd();
                    /* Compare, then Present */
                    compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_BLENDED);
                    renderer.present();
                }

                {
                    /* Test Mod. */
                    test_blit_blend_mode(renderer, tface, SDL3::SDL_BLENDMODE_MOD);
                    sdl::Surface reference_surface = sdl::test::image::ImageBlitBlendMod();
                    /* Compare, then Present */
                    compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_BLENDED);
                    renderer.present();
                }
            }

            /* Clear surface. */
            clear_screen(renderer);

            /* Loop blit. */
            rl::i32 check_fail_count_1{ 0 };
            rl::i32 check_fail_count_2{ 0 };
            rl::i32 check_fail_count_3{ 0 };
            rl::i32 check_fail_count_4{ 0 };

            for (rl::i32 j = 0; j <= nj; j += 4)
            {
                for (rl::i32 i = 0; i <= ni; i += 4)
                {
                    if (!tface.set_color_mod({ (255 / nj) * j, (255 / ni) * i, (255 / nj) * j }))
                        ++check_fail_count_1;

                    if (!tface.set_alpha_mod(cast::to<u8>((100 / ni) * i)))
                        ++check_fail_count_2;

                    /* Crazy blending mode magic. */
                    SDL3::SDL_BlendMode mode(static_cast<SDL3::SDL_BlendMode>((i / 4 * j / 4) % 4));
                    if (mode == 0)
                        mode = SDL3::SDL_BLENDMODE_NONE;
                    else if (mode == 1)
                        mode = SDL3::SDL_BLENDMODE_BLEND;
                    else if (mode == 2)
                        mode = SDL3::SDL_BLENDMODE_ADD;
                    else if (mode == 3)
                        mode = SDL3::SDL_BLENDMODE_MOD;

                    if (!tface.set_blend_mode(mode))
                        ++check_fail_count_3;

                    /* Blitting. */
                    rect.pt.x = (float)i;
                    rect.pt.y = (float)j;
                    if (!renderer.draw_texture(tface, ds::rect<f32>::null(), rect))
                        ++check_fail_count_4;
                }
            }
        }

        /* Check to see if final image matches. */
        sdl::Surface reference_surface = sdl::test::image::ImageBlitBlendAll();
        compare(renderer, reference_surface, ALLOWABLE_ERROR_BLENDED);

        /* Make current */
        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * \brief Test viewport
     */
    static int render_test_viewport(sdl::Renderer& renderer)
    {
        ds::rect<i32> viewport{
            TESTRENDER_SCREEN_W / 3,
            TESTRENDER_SCREEN_H / 3,
            TESTRENDER_SCREEN_W / 2,
            TESTRENDER_SCREEN_H / 2,
        };
        /* Create expected result */
        sdl::Surface reference_surface{
            sdl::test::TESTRENDER_SCREEN_W,
            sdl::test::TESTRENDER_SCREEN_H,
            sdl::test::RENDER_COMPARE_FORMAT,
        };

        bool ret = true;
        sdl::Color fill_color{ 0, 0, 0 };
        rl::u32 color_val = fill_color.rgba(reference_surface.get_format_full());
        runtime_assert(color_val == sdl::test::RENDER_COLOR_CLEAR, "color conversion mismatch");
        ret = reference_surface.fill(sdl::test::RENDER_COLOR_CLEAR);

        fill_color = { 0, 255, 0 };
        color_val = fill_color.rgba(reference_surface.get_format_full());
        runtime_assert(color_val == sdl::test::RENDER_COLOR_GREEN, "color conversion mismatch");
        reference_surface.fill_rect(sdl::test::RENDER_COLOR_GREEN, viewport);

        /* Clear surface. */
        clear_screen(renderer);

        renderer.set_viewport(viewport);
        renderer.set_draw_color({ 0, 255, 0 });
        renderer.fill_rect();
        renderer.set_viewport();

        /* Check to see if final image matches. */
        compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_OPAQUE);

        /*
         * Verify that clear ignores the viewport
         */

        /* Create expected result */
        reference_surface.fill(RENDER_COLOR_GREEN);

        /* Clear surface. */
        clear_screen(renderer);

        /* Set the viewport and do a clear operation */
        renderer.set_viewport(viewport);
        renderer.set_draw_color({ 0, 255, 0 });
        renderer.clear();
        renderer.set_viewport();

        /* Check to see if final image matches. */
        compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_OPAQUE);

        /* Make current */
        renderer.present();

        // SDL3::SDL_DestroySurface(reference_surface);
        runtime_assert(ret, "????");
        return TEST_COMPLETED;
    }

    /**
     * \brief Test logical size
     */
    static int render_test_logical_size(sdl::Renderer& renderer)
    {
        const int factor = 2;
        ds::rect<i32> viewport = {
            ((sdl::test::TESTRENDER_SCREEN_W / 4) / factor) * factor,
            ((sdl::test::TESTRENDER_SCREEN_H / 4) / factor) * factor,
            ((sdl::test::TESTRENDER_SCREEN_W / 2) / factor) * factor,
            ((sdl::test::TESTRENDER_SCREEN_H / 2) / factor) * factor,
        };

        /* Create expected result */
        sdl::Surface reference_surface = {
            sdl::test::TESTRENDER_SCREEN_W,
            sdl::test::TESTRENDER_SCREEN_H,
            sdl::test::RENDER_COMPARE_FORMAT,
        };

        reference_surface.fill(sdl::test::RENDER_COLOR_CLEAR);
        reference_surface.fill_rect(sdl::test::RENDER_COLOR_GREEN, viewport);

        /* Clear surface. */
        clear_screen(renderer);

        /* Set the logical size and do a fill operation */
        ds::dims<rl::i32> out_size = renderer.get_output_size();
        renderer.set_logical_size({ out_size.width / factor, out_size.height / factor },
                                  SDL3::SDL_LOGICAL_PRESENTATION_LETTERBOX,
                                  SDL3::SDL_SCALEMODE_NEAREST);

        out_size = renderer.get_output_size();

        auto logical_size = renderer.set_logical_size(
            { out_size.width / factor, out_size.height / factor },
            SDL3::SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL3::SDL_SCALEMODE_NEAREST);

        renderer.set_draw_color({ 0, 255, 0, Color::Preset::Opaque });

        ds::rect<f32> rect = {
            (float)viewport.pt.x / factor,
            (float)viewport.pt.y / factor,
            (float)viewport.size.width / factor,
            (float)viewport.size.height / factor,
        };

        renderer.fill_rect(rect);
        renderer.set_logical_size({ 0, 0 }, SDL3::SDL_LOGICAL_PRESENTATION_DISABLED,
                                  SDL3::SDL_SCALEMODE_NEAREST);

        /* Check to see if final image matches. */
        compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_OPAQUE);

        /* Clear surface. */
        clear_screen(renderer);

        /* Set the logical size and viewport and do a fill operation */
        out_size = renderer.get_output_size();

        renderer.set_logical_size({ out_size.width / factor, out_size.height / factor },
                                  SDL3::SDL_LOGICAL_PRESENTATION_LETTERBOX,
                                  SDL3::SDL_SCALEMODE_NEAREST);

        viewport = {
            {
                (sdl::test::TESTRENDER_SCREEN_W / 4) / factor,
                (sdl::test::TESTRENDER_SCREEN_H / 4) / factor,
            },
            {
                (sdl::test::TESTRENDER_SCREEN_W / 2) / factor,
                (sdl::test::TESTRENDER_SCREEN_H / 2) / factor,
            },
        };

        renderer.set_viewport(viewport);
        renderer.set_draw_color({ 0, 255, 0 });
        renderer.fill_rect();
        renderer.set_viewport();
        renderer.set_logical_size({ 0, 0 }, SDL3::SDL_LOGICAL_PRESENTATION_DISABLED,
                                  SDL3::SDL_SCALEMODE_NEAREST);

        /* Check to see if final image matches. */
        compare(renderer, reference_surface, sdl::test::ALLOWABLE_ERROR_OPAQUE);

        /*
         * Test a logical size that isn't the same aspect ratio as the window
         */
        viewport = {
            { (sdl::test::TESTRENDER_SCREEN_W / 4), 0 },
            { sdl::test::TESTRENDER_SCREEN_W, sdl::test::TESTRENDER_SCREEN_H },
        };

        /* Create expected result */
        reference_surface.fill(RENDER_COLOR_CLEAR);
        reference_surface.fill_rect(RENDER_COLOR_GREEN, viewport);

        /* Clear surface. */
        clear_screen(renderer);

        /* Set the logical size and do a fill operation */
        out_size = renderer.get_output_size();
        renderer.set_logical_size(
            { out_size.width - 2 * (TESTRENDER_SCREEN_W / 4), out_size.height },
            SDL3::SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL3::SDL_SCALEMODE_LINEAR);

        renderer.set_draw_color({ 0, 255, 0, Color::Preset::Opaque });
        renderer.fill_rect();
        renderer.set_logical_size({ 0, 0 }, SDL3::SDL_LOGICAL_PRESENTATION_DISABLED,
                                  SDL3::SDL_SCALEMODE_NEAREST);

        /* Check to see if final image matches. */
        compare(renderer, reference_surface, ALLOWABLE_ERROR_OPAQUE);

        /* Clear surface. */
        clear_screen(renderer);

        /* Make current */
        renderer.present();

        return TEST_COMPLETED;
    }

    /**
     * @brief Runs the full SDL test suite on C++ wrapper
     * */
    int execute_render_tests(sdl::Window& main_window)
    {
        int ret = 0;

        // if (!main_window.is_valid())
        //     return -1;
        // if (!main_window.renderer()->is_valid())
        //     return -1;

        // std::shared_ptr<sdl::Renderer> window_renderer = main_window.renderer();
        // ret |= render_test_get_num_render_drivers(*window_renderer);
        // ret |= render_test_primitives(*window_renderer);
        // ret |= render_test_primitives_blend(*window_renderer);
        // ret |= render_test_blit(*window_renderer);
        // ret |= render_test_blit_color(*window_renderer);
        // ret |= render_test_blit_alpha(*window_renderer);
        // ret |= render_test_blit_blend(*window_renderer);
        // ret |= render_test_viewport(*window_renderer);
        // ret |= render_test_logical_size(*window_renderer);

        runtime_assert(ret >= 0, "rendering test failure");
        return ret;
    }
}
