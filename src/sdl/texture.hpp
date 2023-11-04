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
    struct SDL_Texture;
    typedef SDL_Texture SDL_Texture;
}

namespace rl::sdl
{
    class texture
    {
    public:
        texture(texture&& tex);

        operator SDL3::SDL_Texture();

        i32 width();
        i32 height();
        SDL3::SDL_Texture* sdl_data();

    protected:
        SDL3::SDL_Texture* m_sdl_texture{};
    };
}
