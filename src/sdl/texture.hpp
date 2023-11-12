#pragma once

#include <condition_variable>
#include <thread>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "sdl/color.hpp"
#include "sdl/window.hpp"

namespace rl::sdl {
    template <typename T>
    class scoped_lock;

    class renderer;
    class surface;

    class texture
    {
    public:
        texture(const sdl::texture& other) = delete;

        texture(SDL3::SDL_Texture*&& other);
        texture(sdl::texture&& other);
        texture(sdl::renderer& renderer, const sdl::surface& surface);
        texture(std::shared_ptr<sdl::renderer> renderer, const sdl::surface& surface);
        texture(std::shared_ptr<sdl::renderer> renderer,
                u32 format = SDL3::SDL_PIXELFORMAT_RGBA8888,
                i32 access = SDL3::SDL_TEXTUREACCESS_TARGET, i32 width = 1024, i32 height = 768);

        ~texture();

        bool is_valid() const;
        SDL3::SDL_Texture* sdl_handle() const;
        i32 query_texture(SDL3::SDL_PixelFormatEnum& format, SDL3::SDL_TextureAccess& access,
                          ds::dimensions<i32>& dims);

        texture& operator=(texture&& other);
        texture& operator=(SDL3::SDL_Texture*&& other) &&;

        bool update(const void* pixels, i32 pitch,
                    const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool update(sdl::surface& surface, const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool update(sdl::surface&& surface, const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool update_yuv(const u8* yplane, i32 ypitch, const u8* uplane, i32 upitch,
                        const u8* vplane, i32 vpitch,
                        const ds::rect<i32>& rect = ds::rect<i32>::null());
        bool set_blend_mode(SDL3::SDL_BlendMode blend_mode);
        bool set_alpha_mod(u8 alpha);
        bool set_color_mod(sdl::color c);
        SDL3::SDL_PixelFormatEnum get_format() const;
        SDL3::SDL_TextureAccess get_access() const;
        ds::dimensions<i32> size();
        u8 get_alpha_mod() const;
        SDL3::SDL_BlendMode get_blend_mode() const;
        sdl::color get_color_mod() const;

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
