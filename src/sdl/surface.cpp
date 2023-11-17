#include <type_traits>

#include "sdl/defs.hpp"
#include "sdl/renderer.hpp"
#include "sdl/scoped_lock.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"
#include "sdl/utils.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    surface::surface(SDL3::SDL_Surface* surface)
        : m_sdl_surface{ surface }
    {
        runtime_assert(m_sdl_surface != nullptr, "faied to construct surface");
        surface = nullptr;
    }

    surface::surface(surface&& other)
        : m_sdl_surface{ other.m_sdl_surface }
    {
        other.m_sdl_surface = nullptr;
    }

    surface::surface(i32 width, i32 height, SDL3::SDL_PixelFormatEnum format)
        : m_sdl_surface{ SDL3::SDL_CreateSurface(width, height, format) }
    {
        runtime_assert(m_sdl_surface != nullptr, "faied to construct surface");
    }

    surface::surface(void* pixels, i32 width, i32 height, i32 pitch,
                     SDL3::SDL_PixelFormatEnum format)
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

    surface& surface::operator=(surface other)
    {
        runtime_assert(other.is_valid(), "assigning invalid surface");
        if (m_sdl_surface != nullptr)
        {
            SDL3::SDL_DestroySurface(m_sdl_surface);
            m_sdl_surface = nullptr;
        }

        if (other.is_valid())
            std::swap(m_sdl_surface, other.m_sdl_surface);

        return *this;
    }

    surface& surface::operator=(surface&& other) noexcept
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

    bool surface::is_valid() const
    {
        return this->sdl_handle() != nullptr;
    }

    void* const& surface::get_pixels() const&
    {
        if (SDL_MUSTLOCK(m_sdl_surface))
            runtime_assert(this->is_locked, "unsafe surface pixels access, not locked");
        return m_sdl_surface->pixels;
    }

    i32 surface::read_pixel(const ds::point<i32>& pt, sdl::color& color)
    {
        ds::rect<i32> surface_rect{ { 0, 0 }, this->size() };
        runtime_assert(this->is_valid(), "attempting to read pixel from invalid surface");
        runtime_assert(m_sdl_surface->format != nullptr,
                       "can't read pixel from surface without defined format");
        runtime_assert(m_sdl_surface->format != nullptr,
                       "attempting to read pixel from surface without pixel data");
        runtime_assert(surface_rect.overlaps(pt),
                       "attempting to read pixed from location outside of surface rect");

        u32 pixel{ 0 };
        void* p{ nullptr };
        u32 bytes_per_pixel = m_sdl_surface->format->BytesPerPixel;
        if (bytes_per_pixel > sizeof(pixel))
            runtime_assert(bytes_per_pixel > sizeof(pixel), "surface->format->BytesPerPixel");
        else
        {
            scoped_lock<sdl::surface> lock(*this);
            void* pixel_addr = static_cast<u8*>(m_sdl_surface->pixels) +
                               pt.y * m_sdl_surface->pitch + pt.x * bytes_per_pixel;

            // Fill the appropriate number of least-significant bytes of pixel,
            // leaving the most-significant bytes set to zero
            SDL3::SDL_memcpy(&pixel, pixel_addr, bytes_per_pixel);
            SDL3::SDL_GetRGBA(pixel, m_sdl_surface->format, &color.r, &color.g, &color.b, &color.a);
        }

        return 0;
    }

    i32 surface::compare(sdl::surface& other, int allowable_error)
    {
        i32 failures = 0;

        // Validate input surfaces
        sdl_assert(this->is_valid(), "can't compare an uninitialized surface");
        if (!this->is_valid())
            return -1;

        sdl_assert(other.is_valid(), "can't compare to uninitialized surface");
        if (other.is_valid())
            return -1;

        const auto t_size{ this->size() };
        const auto o_size{ other.size() };
        if (t_size != o_size)
        {
            sdl_assert(t_size == o_size, "Expected (" << t_size.width << "x" << t_size.height << ")"
                                                      << ", got(" << o_size.width << "x"
                                                      << o_size.height << ")");
            return -2;
        }
        /* Sanitize input value */
        if (allowable_error < 0)
            allowable_error = 0;

        // i32 sample_error_x = 0;
        // i32 sample_error_y = 0;
        // i32 sample_dist = 0;
        sdl::color sample_o_color{ 0, 0, 0, 0 };
        sdl::color sample_t_color{ 0, 0, 0, 0 };

        {
            sdl::scoped_lock<sdl::surface> this_lock(*this);
            sdl::scoped_lock<sdl::surface> other_lock(other);

            // SDL3::SDL_LockSurface(m_sdl_surface);
            // SDL3::SDL_LockSurface(other.m_sdl_surface);

            i32 temp{ 0 };
            i32 dist{ 0 };
            sdl::color t_color{ 0, 0, 0, 0 };
            sdl::color o_color{ 0, 0, 0, 0 };

            /* Compare image - should be same format. */
            for (i32 y = 0; y < t_size.height; y++)
            {
                for (i32 x = 0; x < t_size.width; x++)
                {
                    ds::point<i32> pos{ x, y };
                    temp = this->read_pixel(pos, t_color);
                    sdl_assert(temp == 0,
                               fmt::format("failed to read (t) pixel ({},{})", pos.x, pos.y));

                    if (temp != 0)
                        continue;

                    temp = other.read_pixel(pos, o_color);
                    sdl_assert(temp == 0,
                               fmt::format("failed to read (o) pixel ({},{})", pos.x, pos.y));

                    if (temp != 0)
                        continue;

                    dist = 0;
                    dist += (t_color.r - o_color.r) * (t_color.r - o_color.r);
                    dist += (t_color.g - o_color.g) * (t_color.g - o_color.g);
                    dist += (t_color.b - o_color.b) * (t_color.b - o_color.b);

                    // Allow some difference in blending accuracy
                    if (dist > allowable_error)
                    {
                        failures++;
                        // if (failures == 1)
                        //{
                        //     sample_error_x = x;
                        //     sample_error_y = y;
                        //     sample_dist = dist;
                        //     sample_o_color = o_color;
                        //     sample_t_color = t_color;
                        // }
                    }
                }
            }
        }
        // SDL3::SDL_UnlockSurface(m_sdl_surface);
        // SDL3::SDL_UnlockSurface(other.m_sdl_surface);

        if (failures != 0)
        {
            sdl_assert(
                failures == 0,
                fmt::format("Comparison of pixels with allowable error of {} failed {} times.",
                            allowable_error, failures)
                    .data());
            sdl_assert(failures == 0, fmt::format("Actual surface format type: {}",
                                                  std::to_underlying(this->get_format())));
            sdl_assert(failures == 0, fmt::format("Reference surface format type: {}",
                                                  std::to_underlying(other.get_format())));
        }

        return failures;
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
        return sdl_surface;
    }

    surface surface::convert(u32 pixel_format)
    {
        SDL3::SDL_Surface* sdl_surface = SDL3::SDL_ConvertSurfaceFormat(m_sdl_surface, pixel_format);
        runtime_assert(sdl_surface != nullptr, "failed to convert surface");
        return sdl_surface;
    }

    void surface::blit(surface& dst_surface, ds::rect<i32>& dst_rect)
    {
        i32 result = SDL3::SDL_BlitSurface(m_sdl_surface, nullptr, dst_surface.sdl_handle(),
                                           dst_rect);
        runtime_assert(result == 0, "failed to blit surface");
    }

    void surface::blit_rect(surface& dst_surface, ds::rect<i32>& dst_rect,
                            const ds::rect<i32>& src_rect)
    {
        i32 result = SDL3::SDL_BlitSurface(m_sdl_surface, src_rect, dst_surface.sdl_handle(),
                                           dst_rect);
        runtime_assert(result == 0, "failed to blit surface");
    }

    void surface::blit_scaled_rect(const ds::rect<i32>& src_rect, surface& dst_surface,
                                   ds::rect<i32>& dst_rect)
    {
        i32 result = SDL3::SDL_BlitSurfaceScaled(m_sdl_surface, src_rect, dst_surface.sdl_handle(),
                                                 dst_rect);
        runtime_assert(result == 0, "failed to blit (scaled) surface");
    }

    void surface::blit_scaled(surface& dst_surface, ds::rect<i32>& dst_rect)
    {
        i32 result = SDL3::SDL_BlitSurfaceScaled(m_sdl_surface, nullptr, dst_surface.sdl_handle(),
                                                 dst_rect);
        runtime_assert(result == 0, "failed to blit (scaled) surface");
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
        runtime_assert(result == 0, "failed to get color key");
        return color_key;
    }

    u8 surface::get_alpha_mod() const
    {
        u8 alpha{ 0 };
        i32 result = SDL3::SDL_GetSurfaceAlphaMod(m_sdl_surface, &alpha);
        runtime_assert(result == 0, "failed to get alpha mod");
        return alpha;
    }

    SDL3::SDL_BlendMode surface::get_blend_mode() const
    {
        SDL3::SDL_BlendMode blend_mode{ SDL3::SDL_BLENDMODE_NONE };
        i32 result = SDL3::SDL_GetSurfaceBlendMode(m_sdl_surface, &blend_mode);
        runtime_assert(result == 0, "failed to get blend mode");
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
        runtime_assert(result == 0, "failed to get surface color mod");
    }

    bool surface::set_clip_rect(const ds::rect<i32>& rect)
    {
        SDL3::SDL_bool result = SDL3::SDL_SetSurfaceClipRect(m_sdl_surface, rect);
        runtime_assert(result == sdl::boolean(result), "failed to set clip rect");
        return result == sdl::boolean(result);
    }

    bool surface::set_color_key(bool flag, u32 key)
    {
        i32 result = SDL3::SDL_SetSurfaceColorKey(m_sdl_surface, flag, key);
        runtime_assert(result == 0, "failed to set color key");
        return result == 0;
    }

    bool surface::set_alpha_mod(u8 alpha)
    {
        i32 result = SDL3::SDL_SetSurfaceAlphaMod(m_sdl_surface, alpha);
        runtime_assert(result == 0, "failed to set alpha mod");
        return result == 0;
    }

    bool surface::set_blend_mode(SDL3::SDL_BlendMode blend_mode)
    {
        i32 result = SDL3::SDL_SetSurfaceBlendMode(m_sdl_surface, blend_mode);
        runtime_assert(result == 0, "failed to set blend mode");
        return result == 0;
    }

    bool surface::set_color_mod(sdl::color c)
    {
        i32 result = 0;
        result |= SDL3::SDL_SetSurfaceColorMod(m_sdl_surface, c.r, c.g, c.b);
        runtime_assert(result == 0, "failed to set color mode");
        result |= SDL3::SDL_SetSurfaceAlphaMod(m_sdl_surface, c.a);
        runtime_assert(result == 0, "failed to set color mode");
        return result == 0;
    }

    /**
     * @brief Sets RLE (Run Length Encoding) acceleration for the surface.
     * */
    bool surface::set_rle_acceleration(bool flag)
    {
        i32 result = SDL3::SDL_SetSurfaceRLE(m_sdl_surface, flag ? 1 : 0);
        runtime_assert(result == 0, "failed to set rle accelleration");
        return result == 0;
    }

    bool surface::fill(u32 color)
    {
        i32 result = SDL3::SDL_FillSurfaceRect(m_sdl_surface, nullptr, color);
        runtime_assert(result == 0, "failed to fill surface");
        return result == 0;
    }

    bool surface::fill(const sdl::color& color)
    {
        u32 color_val = color.rgba(m_sdl_surface->format);
        return this->fill(color_val);
    }

    bool surface::fill_rect(u32 color, const ds::rect<i32>& rect)
    {
        i32 result = SDL3::SDL_FillSurfaceRect(m_sdl_surface, rect, color);
        runtime_assert(result == 0, "failed to fill rect");
        return result == 0;
    }

    bool surface::fill_rect(const sdl::color& color, const ds::rect<i32>& rect)
    {
        u32 color_val = color.rgba(m_sdl_surface->format);
        return this->fill_rect(color_val, rect);
    }

    bool surface::fill_rects(u32 color, const std::vector<ds::rect<i32>>& rects)
    {
        bool ret = false;

        i32 count = cast::to<i32>(rects.size());
        if (count > 0)
        {
            i32 result = SDL3::SDL_FillSurfaceRects(m_sdl_surface, rects.front(), count, color);
            runtime_assert(result == 0, "failed to fill rects");
            ret = result == 0;
        }

        return ret;
    }

    bool surface::fill_rects(const sdl::color& color, const std::vector<ds::rect<i32>>& rects)
    {
        u32 color_val = color.rgba(m_sdl_surface->format);
        return this->fill_rects(color_val, rects);
    }

    ds::dimensions<i32> surface::size() const
    {
        runtime_assert(this->is_valid(), "failed getting size of uninitialized surface");
        if (m_sdl_surface != nullptr) [[likely]]
        {
            return {
                m_sdl_surface->w,
                m_sdl_surface->h,
            };
        }
        else
            return {};
    }

    SDL3::SDL_PixelFormatEnum surface::get_format() const
    {
        return static_cast<SDL3::SDL_PixelFormatEnum>(m_sdl_surface->format->format);
    }

    const SDL3::SDL_PixelFormat* surface::get_format_full() const
    {
        return m_sdl_surface->format;
    }
}
