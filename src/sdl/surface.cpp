#include "sdl/renderer.hpp"
#include "sdl/scoped_lock.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"

namespace rl::sdl
{
    surface::surface(SDL3::SDL_Surface* surface)
        : m_sdl_surface(surface)
    {
        runtime_assert(m_sdl_surface != nullptr, "faied to construct surface");
    }

    surface::surface(surface&& other)
        : m_sdl_surface{ other.m_sdl_surface }
    {
        other.m_sdl_surface = nullptr;
    }

    surface::surface(i32 width, i32 height,
                     SDL3::SDL_PixelFormatEnum format /*= SDL3::SDL_PIXELFORMAT_UNKNOWN*/)
        : m_sdl_surface{ SDL3::SDL_CreateSurface(width, height, format) }
    {
        runtime_assert(m_sdl_surface != nullptr, "faied to construct surface");
    }

    surface::surface(void* pixels, i32 width, i32 height, i32 pitch,
                     SDL3::SDL_PixelFormatEnum format /*= SDL3::SDL_PIXELFORMAT_UNKNOWN*/)
        : m_sdl_surface{ SDL3::SDL_CreateSurfaceFrom(pixels, width, height, pitch, format) }
    {
        runtime_assert(m_sdl_surface != nullptr, "faied to construct surface");
    }

    surface::~surface()
    {
        if (m_sdl_surface != nullptr)
        {
            using namespace std::literals;
            std::unique_lock<std::mutex> lock(m_lock);
            if (m_is_unlocked_cv.wait_for(lock, 60s, [&]() {
                    return !this->is_locked;
                }))
            {
                SDL3::SDL_DestroySurface(m_sdl_surface);
            }
        }
    }

    surface& surface::operator=(surface&& other)
    {
#ifndef NDEBUG
        runtime_assert(this != &other, "assigning a surface to itself");
        if (this == &other)
            return *this;
#endif

        if (m_sdl_surface != nullptr)
        {
            SDL3::SDL_DestroySurface(m_sdl_surface);
            m_sdl_surface = nullptr;
        }

        std::swap(m_sdl_surface, other.m_sdl_surface);
        return *this;
    }

    SDL3::SDL_Surface* surface::sdl_handle() const
    {
        return m_sdl_surface;
    }

    void* const& surface::get_pixels() const&
    {
        if (SDL_MUSTLOCK(m_sdl_surface))
            runtime_assert(this->is_locked, "unsafe surface pixels access, not locked");
        return m_sdl_surface->pixels;
    }

    i32& surface::get_pitch()
    {
        if (SDL_MUSTLOCK(m_sdl_surface))
            runtime_assert(this->is_locked, "unsafe surface pixels access, not locked");
        return m_sdl_surface->pitch;
    }

    surface surface::convert(const SDL3::SDL_PixelFormat& format)
    {
        SDL3::SDL_Surface* sdl_surface{ SDL3::SDL_ConvertSurface(m_sdl_surface, &format) };
        runtime_assert(sdl_surface != nullptr, "failed to convert surface");
        return surface(sdl_surface);
    }

    surface surface::convert(u32 pixel_format)
    {
        SDL3::SDL_Surface* sdl_surface{ SDL3::SDL_ConvertSurfaceFormat(m_sdl_surface, pixel_format) };
        runtime_assert(sdl_surface != nullptr, "failed to convert surface");
        return surface(sdl_surface);
    }

    void surface::blit(surface& dst_surface, ds::rect<i32>& dst_rect)
    {
        // SDL3::SDL_Rect& sdl_dst_rectt{ dst_rect };
        i32 result = SDL3::SDL_BlitSurface(m_sdl_surface, nullptr, dst_surface.sdl_handle(),
                                           dst_rect);
        runtime_assert(result != 0, "failed to blit surface");
    }

    void surface::blit_rect(surface& dst_surface, ds::rect<i32>& dst_rect,
                            const ds::rect<i32>& src_rect /*= {}*/)
    {
        i32 result = SDL3::SDL_BlitSurface(m_sdl_surface, src_rect, dst_surface.sdl_handle(),
                                           dst_rect);
        runtime_assert(result != 0, "failed to blit surface");
    }

    void surface::blit_scaled_rect(const ds::rect<i32>& src_rect, surface& dst_surface,
                                   ds::rect<i32>& dst_rect)
    {
        i32 result = SDL3::SDL_BlitSurfaceScaled(m_sdl_surface, src_rect, dst_surface.sdl_handle(),
                                                 dst_rect);
        runtime_assert(result != 0, "failed to blit (scaled) surface");
    }

    void surface::blit_scaled(surface& dst_surface, ds::rect<i32>& dst_rect)
    {
        i32 result = SDL3::SDL_BlitSurfaceScaled(m_sdl_surface, nullptr, dst_surface.sdl_handle(),
                                                 dst_rect);
        runtime_assert(result != 0, "failed to blit (scaled) surface");
    }

    ds::rect<i32> surface::get_clip_rect() const
    {
        SDL3::SDL_Rect sdl_rect{ 0, 0, 0, 0 };
        SDL3::SDL_GetSurfaceClipRect(m_sdl_surface, &sdl_rect);
        return sdl_rect;
    }

    u32 surface::get_color_key() const
    {
        u32 color_key{ 0 };
        i32 result = SDL3::SDL_GetSurfaceColorKey(m_sdl_surface, &color_key);
        runtime_assert(result != 0, "failed to get color key");
        return color_key;
    }

    u8 surface::get_alpha_mod() const
    {
        u8 alpha{ 0 };
        i32 result = SDL3::SDL_GetSurfaceAlphaMod(m_sdl_surface, &alpha);
        runtime_assert(result != 0, "failed to get alpha mod");
        return alpha;
    }

    SDL3::SDL_BlendMode surface::get_blend_mode() const
    {
        SDL3::SDL_BlendMode blend_mode{ SDL3::SDL_BLENDMODE_NONE };
        i32 result = SDL3::SDL_GetSurfaceBlendMode(m_sdl_surface, &blend_mode);
        runtime_assert(result != 0, "failed to get blend mode");
        return blend_mode;
    }

    sdl::color surface::get_color_mod() const
    {
        sdl::color c{ 0, 0, 0, 0 };
        this->get_color_mod(c.r, c.g, c.b);
        c.a = this->get_alpha_mod();
        return c;
    }

    void surface::get_color_mod(u8& r, u8& g, u8& b) const
    {
        i32 result = SDL3::SDL_GetSurfaceColorMod(m_sdl_surface, &r, &g, &b);
        runtime_assert(result != 0, "failed to get surface color mod");
    }

    surface& surface::set_clip_rect(const ds::rect<i32>& rect)
    {
        i32 result = SDL3::SDL_SetSurfaceClipRect(m_sdl_surface, rect);
        runtime_assert(result != sdl::boolean(true), "failed to set clip rect");
        return *this;
    }

    surface& surface::set_color_key(bool flag, u32 key)
    {
        i32 result = SDL3::SDL_SetSurfaceColorKey(m_sdl_surface, flag, key);
        runtime_assert(result != 0, "failed to set color key");
        return *this;
    }

    surface& surface::set_alpha_mod(u8 alpha)
    {
        i32 result = SDL3::SDL_SetSurfaceAlphaMod(m_sdl_surface, alpha);
        runtime_assert(result != 0, "failed to set alpha mod");
        return *this;
    }

    surface& surface::set_blend_mode(SDL3::SDL_BlendMode blendMode)
    {
        i32 result = SDL3::SDL_SetSurfaceBlendMode(m_sdl_surface, blendMode);
        runtime_assert(result != 0, "failed to set blend mode");
        return *this;
    }

    surface& surface::set_color_mod(u8 r, u8 g, u8 b)
    {
        i32 result = SDL3::SDL_SetSurfaceColorMod(m_sdl_surface, r, g, b);
        runtime_assert(result != 0, "failed to set color mode");
        return *this;
    }

    surface& surface::set_color_mod(const sdl::color& c)
    {
        this->set_color_mod(c.r, c.g, c.b);
        this->set_alpha_mod(c.a);
        return *this;
    }

    /**
     * @brief Sets RLE (Run Length Encoding) acceleration for the surface.
     * */
    surface& surface::set_rle_acceleration(bool flag)
    {
        i32 result = SDL3::SDL_SetSurfaceRLE(m_sdl_surface, flag ? 1 : 0);
        runtime_assert(result != 0, "failed to set rle accelleration");
        return *this;
    }

    surface& surface::fill(u32 color)
    {
        i32 result = SDL3::SDL_FillSurfaceRect(m_sdl_surface, nullptr, color);
        runtime_assert(result != 0, "failed to fill surface");
        return *this;
    }

    surface& surface::fill_rect(const ds::rect<i32>& rect, u32 color)
    {
        const SDL3::SDL_Rect& sdl_rect = rect;
        i32 result                     = SDL3::SDL_FillSurfaceRect(m_sdl_surface, &sdl_rect, color);
        runtime_assert(result != 0, "failed to fill rect");
        return *this;
    }

    surface& surface::fill_rects(const ds::rect<i32>* rects, i32 count, u32 color)
    {
        std::vector<SDL3::SDL_Rect> sdl_rects;
        sdl_rects.reserve(static_cast<size_t>(count));
        for (const ds::rect<i32>* r = rects; r != rects + count; ++r)
            sdl_rects.emplace_back(*r);

        i32 result = SDL3::SDL_FillSurfaceRects(m_sdl_surface, sdl_rects.data(), count, color);
        runtime_assert(result != 0, "failed to fill rects");
        return *this;
    }

    ds::dimensions<i32> surface::size() const
    {
        return {
            m_sdl_surface->w,
            m_sdl_surface->h,
        };
    }

    SDL3::SDL_PixelFormatEnum surface::get_format() const
    {
        return static_cast<SDL3::SDL_PixelFormatEnum>(m_sdl_surface->format->format);
    }

}
