#include "sdl/renderer.hpp"
#include "sdl/scoped_lock.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"

namespace SDL3
{
#include <SDL3/SDL_render.h>
}

namespace rl::sdl
{
    texture::texture(SDL3::SDL_Texture* other)
        : m_sdl_texture(other)
    {
        runtime_assert(m_sdl_texture != nullptr, "failed to create texture");
    }

    texture::texture(texture&& other)
        : m_sdl_texture(other.m_sdl_texture)
    {
        runtime_assert(m_sdl_texture != nullptr, "failed to create texture");
        other.m_sdl_texture = nullptr;
    }

    texture::texture(sdl::renderer& renderer, u32 format, i32 accesss, i32 width, i32 height)
        : m_sdl_texture{ SDL3::SDL_CreateTexture(renderer.sdl_handle(), format, accesss, width,
                                                 height) }
    {
        runtime_assert(m_sdl_texture != nullptr, "failed to create texture");
    }

    texture::texture(sdl::renderer& renderer, const sdl::surface& surface)
        : m_sdl_texture{ SDL3::SDL_CreateTextureFromSurface(renderer.sdl_handle(),
                                                            surface.sdl_handle()) }
    {
        runtime_assert(m_sdl_texture != nullptr, "failed to create texture");
    }

    texture::~texture()
    {
        if (m_sdl_texture != nullptr)
        {
            // TODO: change timeout after testing
            using namespace std::literals;
            std::unique_lock<std::mutex> lock{ this->m_lock };
            if (m_is_unlocked_cv.wait_for(lock, 60s, [&]() {
                    return this->is_locked == false;
                }))
            {
                SDL3::SDL_DestroyTexture(m_sdl_texture);
            }
        }
    }

    texture& texture::operator=(texture&& other)
    {
#ifndef NDEBUG
        runtime_assert(this != &other, "texture assigned to itself");
        if (this == &other)
            return *this;
#endif

        if (m_sdl_texture != nullptr)
        {
            SDL3::SDL_DestroyTexture(m_sdl_texture);
            m_sdl_texture = nullptr;
        }
        std::swap(m_sdl_texture, other.m_sdl_texture);
        return *this;
    }

    SDL3::SDL_Texture* texture::sdl_handle()
    {
        return m_sdl_texture;
    }

    texture& texture::update(const void* pixels, i32 pitch, const ds::rect<i32>& rect /*= {}*/)
    {
        i32 result = SDL3::SDL_UpdateTexture(m_sdl_texture, rect, pixels, pitch);
        runtime_assert(result == 0, "_________");
        return *this;
    }

    texture& texture::update(sdl::surface& surf, const ds::rect<i32>& rect /*= {}*/)
    {
        const auto&& this_size{ this->size() };
        ds::rect<i32> real_rect{
            rect.is_null() ? rect : ds::rect<i32>{ 0, 0, this_size.width, this_size.height },
        };

        const auto&& surf_size{ surf.size() };
        real_rect.size.width  = std::min(real_rect.width(), surf_size.width);
        real_rect.size.height = std::min(real_rect.height(), surf_size.height);

        if (this->get_format() == surf.get_format())
        {
            sdl::scoped_lock<sdl::surface>{ surf };
            i32& pitch = surf.get_pitch();
            void* const& pixels{ surf.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
        else
        {
            sdl::surface converted{ surf.convert(this->get_format()) };
            sdl::scoped_lock<sdl::surface>{ converted };
            i32& pitch = converted.get_pitch();
            void* const& pixels{ converted.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
    }

    texture& texture::update(sdl::surface&& surf, const ds::rect<i32>& rect /*= {}*/)
    {
        const auto&& this_size{ this->size() };
        ds::rect<i32> real_rect{
            rect.is_null() ? rect : ds::rect<i32>{ 0, 0, this_size.width, this_size.height },
        };

        const auto&& surf_size{ surf.size() };
        real_rect.size.width  = std::min(real_rect.width(), surf_size.width);
        real_rect.size.height = std::min(real_rect.height(), surf_size.height);

        if (this->get_format() == surf.get_format())
        {
            sdl::scoped_lock<sdl::surface>{ surf };
            i32 pitch{ surf.get_pitch() };
            void* const& pixels{ surf.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
        else
        {
            sdl::surface converted{ surf.convert(this->get_format()) };
            sdl::scoped_lock<sdl::surface>{ converted };
            i32 pitch{ converted.get_pitch() };
            void* const& pixels{ converted.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
    }

    texture& texture::update_yuv(const u8* y_plane, int y_pitch, const u8* u_plane, i32 u_pitch,
                                 const u8* v_plane, int v_pitch, const ds::rect<i32>& rect /*= {}*/)
    {
        i32 result = SDL3::SDL_UpdateYUVTexture(m_sdl_texture, rect, y_plane, y_pitch, u_plane,
                                                u_pitch, v_plane, v_pitch);
        runtime_assert(result == 0, "_________");
        return *this;
    }

    texture& texture::set_blend_mode(SDL3::SDL_BlendMode blend_mode)
    {
        i32 result = SDL3::SDL_SetTextureBlendMode(m_sdl_texture, blend_mode);
        runtime_assert(result == 0, "_________");

        return *this;
    }

    texture& texture::set_alpha_mod(u8 a)
    {
        i32 result = SDL3::SDL_SetTextureAlphaMod(m_sdl_texture, a);
        runtime_assert(result == 0, "_________");
        return *this;
    }

    texture& texture::set_color_mod(u8 r, u8 g, u8 b)
    {
        i32 result = SDL3::SDL_SetTextureColorMod(m_sdl_texture, r, g, b);
        runtime_assert(result == 0, "_________");
        return *this;
    }

    texture& texture::set_color_mod_alpha(const sdl::color& c)
    {
        this->set_color_mod(c.r, c.g, c.b);
        this->set_alpha_mod(c.a);
        return *this;
    }

    SDL3::SDL_PixelFormatEnum texture::get_format() const
    {
        SDL3::SDL_PixelFormatEnum format{ SDL3::SDL_PIXELFORMAT_UNKNOWN };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, reinterpret_cast<u32*>(&format), nullptr,
                                            nullptr, nullptr);
        runtime_assert(result == 0, "_________");
        return format;
    }

    SDL3::SDL_TextureAccess texture::get_access() const
    {
        SDL3::SDL_TextureAccess access{ SDL3::SDL_TEXTUREACCESS_STATIC };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, nullptr, reinterpret_cast<i32*>(&access),
                                            nullptr, nullptr);
        runtime_assert(result == 0, "_________");
        return static_cast<SDL3::SDL_TextureAccess>(access);
    }

    ds::dimensions<i32> texture::size()
    {
        ds::dimensions<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, nullptr, nullptr, &size.width,
                                            &size.height);
        runtime_assert(result != 0, "reading from uninitialized texture");
        return size;
    }

    u8 texture::get_alpha_mod() const
    {
        u8 alpha;
        i32 result = SDL3::SDL_GetTextureAlphaMod(m_sdl_texture, &alpha);
        runtime_assert(result == 0, "_________");

        return alpha;
    }

    SDL3::SDL_BlendMode texture::get_blend_mode() const
    {
        SDL3::SDL_BlendMode mode;
        i32 result = SDL3::SDL_GetTextureBlendMode(m_sdl_texture, &mode);
        runtime_assert(result == 0, "_________");

        return mode;
    }

    sdl::color texture::get_color_mod_rgb() const
    {
        color c{ 0, 0, 0, 0 };
        i32 result = SDL3::SDL_GetTextureColorMod(m_sdl_texture, &c.r, &c.g, &c.b);
        runtime_assert(result == 0, "_________");
        return c;
    }

    sdl::color texture::get_color_mod_alpha() const
    {
        sdl::color c{ this->get_color_mod_rgb() };
        c.a = this->get_alpha_mod();
        return c;
    }
}
