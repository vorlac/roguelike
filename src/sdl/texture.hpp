#pragma once

#include <condition_variable>
#include <thread>

#include "core/assert.hpp"
#include "core/main_window.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
SDL_C_LIB_END

namespace rl::sdl {
    template <typename T>
    class scoped_lock;

    class Renderer;
    class Surface;

    class Texture
    {
    public:
        Texture() = default;
        Texture(const sdl::Texture& other) = delete;

        Texture(SDL3::SDL_Texture* other);
        Texture(sdl::Texture&& other) noexcept;
        Texture(sdl::Renderer& renderer, const sdl::Surface& surface);
        Texture(std::shared_ptr<sdl::Renderer> renderer, const sdl::Surface& surface);
        Texture(std::shared_ptr<sdl::Renderer> renderer,
                u32 format = SDL3::SDL_PIXELFORMAT_RGBA8888,
                i32 access = SDL3::SDL_TEXTUREACCESS_TARGET, i32 width = 1024, i32 height = 768);

        ~Texture();

        bool is_valid() const;
        SDL3::SDL_Texture* sdl_handle() const;
        i32 query_texture(SDL3::SDL_PixelFormatEnum& format, SDL3::SDL_TextureAccess& access,
                          ds::dims<i32>& dims);

        Texture& operator=(Texture&& other) noexcept;
        Texture& operator=(SDL3::SDL_Texture* other);

        bool update(const void* pixels, i32 pitch,
                    const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool update(sdl::Surface& surface, const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool update(sdl::Surface&& surface, const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool update_yuv(const u8* yplane, i32 ypitch, const u8* uplane, i32 upitch,
                        const u8* vplane, i32 vpitch,
                        const ds::rect<i32>& rect = ds::rect<i32>::null());

        bool set_blend_mode(SDL3::SDL_BlendMode blend_mode);
        bool set_alpha_mod(u8 alpha);
        bool set_color_mod(ds::color<u8> c);

        SDL3::SDL_PixelFormatEnum get_format() const;
        SDL3::SDL_TextureAccess get_access() const;
        SDL3::SDL_BlendMode get_blend_mode() const;

        // ds::color<u8> get_color_mod() const;
        ds::dims<i32> size();
        u8 get_alpha_mod() const;

    private:
        template <typename T>
        friend class scoped_lock;
        std::mutex m_lock{ std::mutex{} };
        std::condition_variable m_is_unlocked_cv{};
        std::atomic_bool is_locked{ false };

    private:
        SDL3::SDL_Texture* m_sdl_texture{ nullptr };
    };
}
