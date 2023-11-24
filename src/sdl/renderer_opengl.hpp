#pragma once

#include <bitset>
#include <string_view>
#include <utility>
#include <vector>

#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "sdl/color.hpp"
#include "sdl/defs.hpp"
#include "sdl/texture.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    class Window;

    class RendererGL
    {
    public:
        struct Properties : public std::bitset<32>
        {
            using sdl_type = SDL3::SDL_RendererFlags;

            enum Flag : std::underlying_type_t<sdl_type> {
                None = 0,
                Software = SDL3::SDL_RENDERER_SOFTWARE,
                HWAccelerated = SDL3::SDL_RENDERER_ACCELERATED,
                VSync = SDL3::SDL_RENDERER_PRESENTVSYNC,
            };

            constexpr inline operator sdl_type()
            {
                return static_cast<sdl_type>(this->to_ulong());
            }
        };

        struct BlendMode
        {
            using type = SDL3::SDL_BlendMode;
            constexpr static inline auto None = SDL3::SDL_BLENDMODE_NONE;
            constexpr static inline auto Blend = SDL3::SDL_BLENDMODE_BLEND;
            constexpr static inline auto Add = SDL3::SDL_BLENDMODE_ADD;
            constexpr static inline auto Mod = SDL3::SDL_BLENDMODE_MOD;
            constexpr static inline auto Mul = SDL3::SDL_BLENDMODE_MUL;
            constexpr static inline auto Invalid = SDL3::SDL_BLENDMODE_INVALID;
        };

        constexpr static inline Properties DEFAULT_PROPERTY_FLAGS = {
            Properties::HWAccelerated | Properties::VSync,
        };

        explicit RendererGL() = delete;
        explicit RendererGL(RendererGL& other) = delete;
        explicit RendererGL(const RendererGL& other) = delete;

    public:
        explicit RendererGL(const sdl::Window& window, std::string_view driver,
                            RendererGL::Properties flags);

        bool present()
        {
            i32 result = 0;
            return result == 0;
        }

        bool clear(const sdl::Color& c = { 0, 0, 0 })
        {
            bool ret = true;

            return ret;
        }

        ds::dims<i32> get_output_size() const
        {
            ds::dims<i32> s{ 0, 0 };

            return s;
        }

        bool set_draw_color(const sdl::Color& c)
        {
            i32 result = 0;
            return result == 0;
        }

        bool draw_texture(sdl::Texture& texture,
                          const ds::rect<f32>& src_rect = ds::rect<f32>::null(),
                          const ds::rect<f32>& dst_rect = ds::rect<f32>::null())
        {
            i32 result = 0;
            return result == 0;
        }

        bool set_target()
        {
            i32 result = 0;
            return result == 0;
        }

        bool set_target(sdl::Texture& tex)
        {
            i32 result = 0;
            return result == 0;
        }

        bool set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode)
        {
            i32 result = 0;
            return result == 0;
        }

        bool draw_point(const ds::point<f32>& pt)
        {
            i32 result = 0;
            return result == 0;
        }

        ds::rect<i32> get_viewport()
        {
            ds::rect<i32> rect{ 0, 0, 0, 0 };

            return rect;
        }

        bool draw_points(const std::vector<ds::point<f32>>& points)
        {
            i32 result = 0;

            return result == 0;
        }

        bool draw_line(const ds::point<f32>& pt1, const ds::point<f32>& pt2)
        {
            i32 result = 0;
            return result == 0;
        }

        bool draw_lines(const std::vector<ds::point<f32>>& lines)
        {
            i32 result = 0;

            return result == 0;
        }

        bool draw_rect(ds::rect<f32>&& rect, const sdl::Color& c = {})
        {
            i32 result = 0;

            return result == 0;
        }

        bool draw_rects(const std::vector<ds::rect<f32>>& rects)
        {
            i32 result = 0;

            return result == 0;
        }

        bool fill_rect(const ds::rect<f32>& rect = ds::rect<i32>::null(), const sdl::Color& c = {})
        {
            i32 result = 0;

            return result == 0;
        }

        bool fill_rects(const std::vector<ds::rect<f32>>& rects, const sdl::Color& c = {})
        {
            i32 result = 0;

            return result == 0;
        }

        bool fill_rects(const std::vector<std::pair<ds::rect<f32>, sdl::Color>>& rects)
        {
            bool ret = true;

            return ret;
        }

    private:
        Properties m_properties{ Properties::None };
        SDL3::SDL_GLContext m_sdl_glcontext{ nullptr };
    };
}
