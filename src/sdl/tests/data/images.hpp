#pragma once

#include "sdl/defs.hpp"
#include "utils/numeric.hpp"

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

    rl::sdl::Surface ImageBlit();
    rl::sdl::Surface ImageBlitColor();
    rl::sdl::Surface ImageBlitAlpha();
    rl::sdl::Surface ImageBlitBlendAdd();
    rl::sdl::Surface ImageBlitBlend();
    rl::sdl::Surface ImageBlitBlendMod();
    rl::sdl::Surface ImageBlitBlendNone();
    rl::sdl::Surface ImageBlitBlendAll();
    rl::sdl::Surface ImageFace();
    rl::sdl::Surface ImagePrimitives();
    rl::sdl::Surface ImagePrimitivesBlend();
    rl::sdl::Surface ImagePrimitivesBlendSurface();
}
