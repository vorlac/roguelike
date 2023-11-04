

#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
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

    SDL3::SDL_Texture* texture::sdl_data()
    {
        return m_sdl_texture;
    }

    i32 texture::width()
    {
        runtime_assert(m_sdl_texture != nullptr, "reading from uninitialized texture");
        return m_sdl_texture ? m_sdl_texture->w : 0;
    }

    i32 texture::height()
    {
        runtime_assert(m_sdl_texture != nullptr, "reading from uninitialized texture");
        return m_sdl_texture ? m_sdl_texture->h : 0;
    }
}
