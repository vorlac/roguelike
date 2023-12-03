#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

#include "core/numeric.hpp"
#include "ds/color.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "sdl/pixel_data.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_pixels.h>
struct SDL_Surface;
SDL_C_LIB_END

namespace rl::sdl {
    template <typename T>
    class scoped_lock;

    class Surface
    {
    public:
        explicit Surface(Surface&& other);
        Surface(SDL3::SDL_Surface* surface);

        Surface(i32 width, i32 height,
                SDL3::SDL_PixelFormatEnum format = SDL3::SDL_PIXELFORMAT_UNKNOWN);
        Surface(void* pixels, i32 width, i32 height, i32 pitch,
                SDL3::SDL_PixelFormatEnum format = SDL3::SDL_PIXELFORMAT_UNKNOWN);

        Surface(const Surface& other) = delete;

        ~Surface();

        Surface& operator=(sdl::Surface&& other) noexcept;
        Surface& operator=(sdl::Surface other);

        i32 read_pixel(const ds::point<i32>& pt, ds::color<u8>& color);
        i32 compare(sdl::Surface& other, int allowable_error = 0);
        bool is_valid() const;
        SDL3::SDL_Surface* sdl_handle() const;
        void* const& get_pixels() const&;
        i32& get_pitch();
        Surface convert(const SDL3::SDL_PixelFormat& format);
        Surface convert(u32 pixel_format);

        void blit(Surface& dst_surface, ds::rect<i32>& dst_rect);
        void blit_rect(Surface& dst_surface, ds::rect<i32>& dst_rect,
                       const ds::rect<i32>& src_rect = ds::rect<i32>::null());
        void blit_scaled_rect(const ds::rect<i32>& src_rect, Surface& dst_surface,
                              ds::rect<i32>& dst_rect);
        void blit_scaled(Surface& dst_surface, ds::rect<i32>& dst_rect);

        ds::rect<i32> get_clip_rect() const;
        u32 get_color_key() const;
        u8 get_alpha_mod() const;
        SDL3::SDL_BlendMode get_blend_mode() const;
        ds::color<u8> get_color_mod() const;
        bool get_color_mod(u8& r, u8& g, u8& b) const;
        bool set_clip_rect(const ds::rect<i32>& rect);
        bool set_color_key(bool flag, u32 key);
        bool set_alpha_mod(u8 alpha);
        bool set_blend_mode(SDL3::SDL_BlendMode blend_mode);
        bool set_color_mod(ds::color<u8> c);
        bool set_rle_acceleration(bool flag);
        bool fill(u32 color);
        bool fill(const ds::color<u8>& color);
        bool fill_rect(u32 color, const ds::rect<i32>& rect);
        bool fill_rect(const ds::color<u8>& color, const ds::rect<i32>& rect);
        bool fill_rects(u32 color, const std::vector<ds::rect<i32>>& rects);
        bool fill_rects(const ds::color<u8>& color, const std::vector<ds::rect<i32>>& rects);
        ds::dims<i32> size() const;
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
        sdl::PixelData m_pixel_data{};
    };
}
