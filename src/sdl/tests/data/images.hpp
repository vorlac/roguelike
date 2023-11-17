#pragma once

#include "core/numeric_types.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_pixels.h>
SDL_C_LIB_END

namespace rl::sdl::test::image {
    class surface;

    struct SurfaceImage
    {
        rl::i32 width{ 0 };
        rl::i32 height{ 0 };
        rl::u32 bytes_per_pixel{ 0 }; /* 3:RGB, 4:RGBA */
        const char* pixel_data{ nullptr };
    };

    rl::sdl::surface ImageBlit();
    rl::sdl::surface ImageBlitColor();
    rl::sdl::surface ImageBlitAlpha();
    rl::sdl::surface ImageBlitBlendAdd();
    rl::sdl::surface ImageBlitBlend();
    rl::sdl::surface ImageBlitBlendMod();
    rl::sdl::surface ImageBlitBlendNone();
    rl::sdl::surface ImageBlitBlendAll();
    rl::sdl::surface ImageFace();
    rl::sdl::surface ImagePrimitives();
    rl::sdl::surface ImagePrimitivesBlend();
    rl::sdl::surface ImagePrimitivesBlendSurface();
}
