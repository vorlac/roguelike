#pragma once

#include <vector>

#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "sdl/color.hpp"

namespace SDL3
{
    struct SDL_Surface;

#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_pixels.h>
}

namespace rl::sdl
{
    template <typename T>
    class scoped_lock;

    class surface
    {
        surface(const surface& other) = delete;

    public:
        surface(SDL3::SDL_Surface* surface);
        explicit surface(surface&& other);

        surface(i32 width, i32 height,
                SDL3::SDL_PixelFormatEnum format = SDL3::SDL_PIXELFORMAT_UNKNOWN);
        surface(void* pixels, i32 width, i32 height, i32 pitch,
                SDL3::SDL_PixelFormatEnum format = SDL3::SDL_PIXELFORMAT_UNKNOWN);

        ~surface();

        surface& operator=(surface&& other);
        surface& operator=(surface other);

        i32 read_pixel(const ds::point<i32>& pt, sdl::color& color);
        i32 compare(sdl::surface& other, int allowable_error = 0);
        bool is_valid() const;
        SDL3::SDL_Surface* sdl_handle() const;
        void* const& get_pixels() const&;
        i32& get_pitch();
        surface convert(const SDL3::SDL_PixelFormat& format);
        surface convert(u32 pixel_format);

        void blit(surface& dst_surface, ds::rect<i32>& dst_rect);
        void blit_rect(surface& dst_surface, ds::rect<i32>& dst_rect,
                       const ds::rect<i32>& src_rect = ds::rect<i32>::null());
        void blit_scaled_rect(const ds::rect<i32>& src_rect, surface& dst_surface,
                              ds::rect<i32>& dst_rect);
        void blit_scaled(surface& dst_surface, ds::rect<i32>& dst_rect);

        ds::rect<i32> get_clip_rect() const;
        u32 get_color_key() const;
        u8 get_alpha_mod() const;
        SDL3::SDL_BlendMode get_blend_mode() const;
        sdl::color get_color_mod() const;
        void get_color_mod(u8& r, u8& g, u8& b) const;
        bool set_clip_rect(const ds::rect<i32>& rect);
        bool set_color_key(bool flag, u32 key);
        bool set_alpha_mod(u8 alpha);
        bool set_blend_mode(SDL3::SDL_BlendMode blend_mode);
        bool set_color_mod(sdl::color c);
        bool set_rle_acceleration(bool flag);
        bool fill(u32 color);
        bool fill(const sdl::color& color);
        bool fill_rect(u32 color, const ds::rect<i32>& rect);
        bool fill_rect(const sdl::color& color, const ds::rect<i32>& rect);
        bool fill_rects(u32 color, const std::vector<ds::rect<i32>>& rects);
        bool fill_rects(const sdl::color& color, const std::vector<ds::rect<i32>>& rects);
        ds::dimensions<i32> size() const;
        SDL3::SDL_PixelFormatEnum get_format() const;
        const SDL3::SDL_PixelFormat* get_format_full() const;

    private:
        template <typename T>
        friend class scoped_lock;

        std::mutex m_lock{ std::mutex{} };
        std::condition_variable m_is_unlocked_cv{};
        std::atomic_bool is_locked{ false };

    private:
        SDL3::SDL_Surface* m_sdl_surface{ nullptr };
    };
}
