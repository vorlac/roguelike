#pragma once

#include <bitset>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/triangle.hpp"
#include "sdl/defs.hpp"
#include "sdl/texture.hpp"
#include "utils/io.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl {
    class Window;

    class Renderer
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

        constexpr static inline Renderer::Properties DEFAULT_PROPERTY_FLAGS = {
            Renderer::Properties::HWAccelerated
        };

    private:
        explicit Renderer() = delete;
        explicit Renderer(const rl::Renderer& other) = delete;
        explicit Renderer(rl::Renderer& other) = delete;

    public:
        explicit Renderer(rl::Window& window, rl::Renderer::Properties flags);
        ~Renderer() = default;

        ds::dims<i32> get_output_size() const;
        ds::rect<i32> get_viewport() const;

        SDL3::SDL_GLContext gl_context() const
        {
            return m_sdl_glcontext;
        }

        bool set_viewport(const ds::rect<i32>& rect);
        bool set_draw_color(const ds::color<u8>& c);
        bool clear(const ds::color<u8>& c = { 29, 32, 39 });
        bool present();
        bool swap_buffers(rl::Window& window);
        bool set_target();
        bool set_target(sdl::Texture& tex);
        bool set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode);

    private:
        rl::Renderer::Properties m_properties{ Properties::None };
        SDL3::SDL_GLContext m_sdl_glcontext{ nullptr };
    };
}
