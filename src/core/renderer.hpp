#pragma once

#include <bitset>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

// #include <nanovg_gl_utils.h>

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

struct NVGLUframebuffer;

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

    public:
        explicit Renderer(rl::Window& window, rl::Renderer::Properties flags);
        ~Renderer() = default;

        SDL3::SDL_GLContext gl_context() const;
        NVGcontext* nvg_context();

        bool clear(ds::color<f32> c);
        bool swap_buffers(rl::Window& window);

        ds::dims<i32> get_output_size() const;
        ds::rect<i32> get_viewport() const;

        bool set_viewport(const ds::rect<i32>& rect);
        bool set_draw_color(ds::color<f32> c);
        bool set_target();
        bool set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode);

    private:
        explicit Renderer() = delete;
        explicit Renderer(const rl::Renderer& other) = delete;
        explicit Renderer(rl::Renderer& other) = delete;

    private:
        friend static NVGcontext* create_nanovg_context(rl::Renderer* renderer);
        constexpr static inline bool GuiWidgetDiagnostics{ true };
        constexpr static inline bool NanoVGDiagnostics{ true };

        rl::Renderer::Properties m_properties{ Properties::None };
        SDL3::SDL_GLContext m_sdl_glcontext{ nullptr };
        NVGcontext* m_nvg_context{ nullptr };
        bool m_stencil_buffer{ false };
    };
}
