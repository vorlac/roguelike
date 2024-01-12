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
    class VectorizedRenderer;

    class OpenGLRenderer
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

        constexpr static inline OpenGLRenderer::Properties DEFAULT_PROPERTY_FLAGS = {
            OpenGLRenderer::Properties::HWAccelerated
        };

    public:
        explicit OpenGLRenderer(rl::Window& window, OpenGLRenderer::Properties flags);
        ~OpenGLRenderer() = default;

        SDL3::SDL_GLContext gl_context() const;
        NVGcontext* nvg_context();

        bool clear();
        bool swap_buffers(rl::Window& window);

        ds::dims<i32> get_output_size() const;
        ds::rect<i32> get_viewport() const;

        bool set_viewport(const ds::rect<i32>& rect);
        bool set_draw_color(ds::color<f32> c);
        bool set_target();
        bool set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode);

    public:
        const std::unique_ptr<VectorizedRenderer>& vectorized_renderer() const
        {
            return m_nvg_renderer;
        }

        void draw_rect_outline(ds::rect<i32> rect, f32 stroke_width, ds::color<f32> color,
                               ui::Outline outline_type)
        {
            m_nvg_renderer->draw_rect_outline(
                std::forward<ds::rect<i32>>(rect), std::forward<f32>(stroke_width),
                std::forward<ds::color<f32>>(color), std::forward<ui::Outline>(outline_type));
        }

    private:
        explicit OpenGLRenderer() = delete;
        explicit OpenGLRenderer(const OpenGLRenderer& other) = delete;
        explicit OpenGLRenderer(OpenGLRenderer& other) = delete;

    private:
        constexpr static inline bool GuiWidgetDiagnostics{ true };
        constexpr static inline bool NanoVGDiagnostics{ true };
        constexpr static inline ds::color<f32> m_background_color{ rl::Colors::Background };
        // constexpr static inline ds::color<f32> m_background_color{ 29, 32, 39 };

        rl::OpenGLRenderer::Properties m_properties{ Properties::None };
        SDL3::SDL_GLContext m_sdl_glcontext{ nullptr };
        std::unique_ptr<rl::VectorizedRenderer> m_nvg_renderer{ nullptr };
    };
}
