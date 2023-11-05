

#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "sdl/renderer.hpp"
#include "sdl/texture.hpp"

namespace SDL3
{
#include <SDL3/SDL_render.h>
}

namespace rl::sdl
{
    texture::texture(texture&& other)
        : m_sdl_texture(other.m_sdl_texture)
    {
        runtime_assert(other.m_sdl_texture != nullptr, "failed to create texture");
        other.m_sdl_texture = nullptr;
    }

    texture::texture(sdl::renderer& renderer, u32 format, i32 accesss, i32 width, i32 height)
        : m_sdl_texture{ SDL3::SDL_CreateTexture(renderer.sdl_handle(), format, accesss, width,
                                                 height) }
    {
    }

    SDL3::SDL_Texture* texture::sdl_handle()
    {
        return m_sdl_texture;
    }

    ds::dimensions<i32> texture::size()
    {
        ds::dimensions<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, nullptr, nullptr, &size.width,
                                            &size.height);
        runtime_assert(result != 0, "reading from uninitialized texture");
        return size;
    }
}
