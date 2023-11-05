#pragma once

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "sdl/color.hpp"
#include "sdl/window.hpp"

namespace SDL3
{
#include <SDL3/SDL_stdinc.h>
    //
#include <SDL3/SDL_blendmode.h>
    //
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_syswm.h>
}

namespace rl::sdl
{
    class renderer;

    class texture
    {
    public:
        texture(texture&& tex);
        texture(sdl::renderer& renderer, u32 format = SDL3::SDL_PIXELFORMAT_RGBA8888,
                i32 accesss = SDL3::SDL_TEXTUREACCESS_TARGET, i32 width = 1024, i32 height = 768);

        ds::dimensions<i32> size();
        SDL3::SDL_Texture* sdl_handle();

    protected:
        SDL3::SDL_Texture* m_sdl_texture{};
    };
}
