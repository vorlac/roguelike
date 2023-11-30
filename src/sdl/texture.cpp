#include "sdl/defs.hpp"
#include "sdl/renderer.hpp"
#include "sdl/scoped_lock.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

namespace rl::sdl {
    Texture::Texture(SDL3::SDL_Texture* other)
        : m_sdl_texture(other)
    {
        sdl_assert(m_sdl_texture != nullptr, "failed to create texture");
        other = nullptr;
    }

    Texture::Texture(Texture&& other) noexcept
        : m_sdl_texture(other.m_sdl_texture)
    {
        sdl_assert(m_sdl_texture != nullptr, "failed to create texture");
        other.m_sdl_texture = nullptr;
    }

    Texture::Texture(std::shared_ptr<sdl::Renderer> renderer, u32 format, i32 accesss, i32 width,
                     i32 height)
        : m_sdl_texture{ SDL3::SDL_CreateTexture(renderer->sdl_handle(), format, accesss, width,
                                                 height) }
    {
        sdl_assert(m_sdl_texture != nullptr, "failed to create texture");
    }

    Texture::Texture(sdl::Renderer& renderer, const sdl::Surface& surface)
        : m_sdl_texture{ SDL3::SDL_CreateTextureFromSurface(renderer.sdl_handle(),
                                                            surface.sdl_handle()) }
    {
    }

    Texture::Texture(std::shared_ptr<sdl::Renderer> renderer, const sdl::Surface& surface)
        : m_sdl_texture{ SDL3::SDL_CreateTextureFromSurface(renderer->sdl_handle(),
                                                            surface.sdl_handle()) }
    {
        sdl_assert(m_sdl_texture != nullptr, "failed to create texture");
    }

    Texture::~Texture()
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
                runtime_assert(!this->is_locked.load(std::memory_order_relaxed),
                               "deleting locked texture");
                SDL3::SDL_DestroyTexture(m_sdl_texture);
            }
        }
    }

    Texture& Texture::operator=(SDL3::SDL_Texture* other)
    {
        if (m_sdl_texture != nullptr)
        {
            SDL3::SDL_DestroyTexture(m_sdl_texture);
            m_sdl_texture = nullptr;
        }

        sdl_assert(other != nullptr, "null texture assignment");
        if (other != nullptr)
        {
            this->m_sdl_texture = other;
            other = nullptr;
        }

        return *this;
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
#ifndef NDEBUG
        sdl_assert(this != &other, "texture assigned to itself");
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

    bool Texture::is_valid() const
    {
        return this->sdl_handle() != nullptr;
    }

    SDL3::SDL_Texture* Texture::sdl_handle() const
    {
        return m_sdl_texture;
    }

    i32 Texture::query_texture(SDL3::SDL_PixelFormatEnum& format, SDL3::SDL_TextureAccess& access,
                               ds::dims<i32>& dims)
    {
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, reinterpret_cast<u32*>(&format),
                                            reinterpret_cast<i32*>(&access), &dims.width,
                                            &dims.height);

        sdl_assert(result == 0, "failed to query texture");
        return result == 0;
    }

    bool Texture::update(const void* pixels, i32 pitch, const ds::rect<i32>& rect /*= {}*/)
    {
        i32 result = SDL3::SDL_UpdateTexture(m_sdl_texture, rect, pixels, pitch);
        sdl_assert(result == 0, "failed to update texture");
        return result == 0;
    }

    bool Texture::update(sdl::Surface& surf, const ds::rect<i32>& rect /*= {}*/)
    {
        const auto&& this_size{ this->size() };
        ds::rect<i32> real_rect{
            rect.is_null() ? rect : ds::rect<i32>{ 0, 0, this_size.width, this_size.height },
        };

        const auto&& surf_size{ surf.size() };
        real_rect.size.width = std::min(real_rect.size.width, surf_size.width);
        real_rect.size.height = std::min(real_rect.size.height, surf_size.height);

        if (this->get_format() == surf.get_format())
        {
            sdl::scoped_lock<sdl::Surface> lock{ surf };
            i32& pitch = surf.get_pitch();
            void* const& pixels{ surf.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
        else
        {
            sdl::Surface converted{ surf.convert(this->get_format()) };
            sdl::scoped_lock<sdl::Surface> lock{ converted };
            i32& pitch = converted.get_pitch();
            void* const& pixels{ converted.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
    }

    bool Texture::update(sdl::Surface&& surf, const ds::rect<i32>& rect /*= {}*/)
    {
        const auto&& this_size{ this->size() };
        ds::rect<i32> real_rect{
            rect.is_null() ? rect : ds::rect<i32>{ 0, 0, this_size.width, this_size.height },
        };

        const auto&& surf_size{ surf.size() };
        real_rect.size.width = std::min(real_rect.size.width, surf_size.width);
        real_rect.size.height = std::min(real_rect.size.height, surf_size.height);

        if (this->get_format() == surf.get_format())
        {
            sdl::scoped_lock<sdl::Surface> lock{ surf };
            i32 pitch{ surf.get_pitch() };
            void* const& pixels{ surf.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
        else
        {
            sdl::Surface converted{ surf.convert(this->get_format()) };
            sdl::scoped_lock<sdl::Surface> lock{ converted };
            i32 pitch{ converted.get_pitch() };
            void* const& pixels{ converted.get_pixels() };
            return this->update(pixels, pitch, real_rect);
        }
    }

    bool Texture::update_yuv(const u8* y_plane, int y_pitch, const u8* u_plane, i32 u_pitch,
                             const u8* v_plane, int v_pitch, const ds::rect<i32>& rect)
    {
        i32 result = SDL3::SDL_UpdateYUVTexture(m_sdl_texture, rect, y_plane, y_pitch, u_plane,
                                                u_pitch, v_plane, v_pitch);
        sdl_assert(result == 0, "failed to update YUV");
        return result == 0;
    }

    bool Texture::set_blend_mode(SDL3::SDL_BlendMode blend_mode)
    {
        i32 result = SDL3::SDL_SetTextureBlendMode(m_sdl_texture, blend_mode);
        sdl_assert(result == 0, "failed to set blend mode");
        return result == 0;
    }

    bool Texture::set_alpha_mod(u8 a)
    {
        i32 result = SDL3::SDL_SetTextureAlphaMod(m_sdl_texture, a);
        sdl_assert(result == 0, "failed to set alpha mod");
        return result == 0;
    }

    bool Texture::set_color_mod(sdl::Color<u8>c)
    {
        i32 result = 0;
        result |= SDL3::SDL_SetTextureColorMod(m_sdl_texture, c.r, c.g, c.b);
        sdl_assert(result == 0, "failed to set color mod");
        result |= SDL3::SDL_SetTextureAlphaMod(m_sdl_texture, c.a);
        sdl_assert(result == 0, "failed to set alpha mod");
        return result == 0;
    }

    SDL3::SDL_PixelFormatEnum Texture::get_format() const
    {
        SDL3::SDL_PixelFormatEnum format{ SDL3::SDL_PIXELFORMAT_UNKNOWN };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, reinterpret_cast<u32*>(&format), nullptr,
                                            nullptr, nullptr);
        sdl_assert(result == 0, "failed to get format");
        return format;
    }

    SDL3::SDL_TextureAccess Texture::get_access() const
    {
        SDL3::SDL_TextureAccess access{ SDL3::SDL_TEXTUREACCESS_STATIC };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, nullptr, reinterpret_cast<i32*>(&access),
                                            nullptr, nullptr);
        sdl_assert(result == 0, "failed to get access");
        return static_cast<SDL3::SDL_TextureAccess>(access);
    }

    ds::dims<i32> Texture::size()
    {
        ds::dims<i32> size{ 0, 0 };
        i32 result = SDL3::SDL_QueryTexture(m_sdl_texture, nullptr, nullptr, &size.width,
                                            &size.height);
        sdl_assert(result != 0, "reading from uninitialized texture");
        return size;
    }

    u8 Texture::get_alpha_mod() const
    {
        u8 alpha = 0;
        i32 result = SDL3::SDL_GetTextureAlphaMod(m_sdl_texture, &alpha);
        sdl_assert(result == 0, "failed to get alpha mod");
        return alpha;
    }

    SDL3::SDL_BlendMode Texture::get_blend_mode() const
    {
        SDL3::SDL_BlendMode mode;
        i32 result = SDL3::SDL_GetTextureBlendMode(m_sdl_texture, &mode);
        sdl_assert(result == 0, "failed to get blend mode");

        return mode;
    }

    sdl::Color<u8>Texture::get_color_mod() const
    {
        i32 result = 0;
        sdl::Color<u8>c{ 0, 0, 0 };
        result |= SDL3::SDL_GetTextureColorMod(m_sdl_texture, &c.r, &c.g, &c.b);
        sdl_assert(result == 0, "failed to get color mod");
        result |= SDL3::SDL_GetTextureAlphaMod(m_sdl_texture, &c.a);
        sdl_assert(result == 0, "failed to get alpha mod");
        return c;
    }
}
