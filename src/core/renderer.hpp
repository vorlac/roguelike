#pragma once

#include "sdl/texture.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

struct NVGLUframebuffer;

namespace rl {
    class MainWindow;

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

        constexpr static inline OpenGLRenderer::Properties DefaultProperties = {
            OpenGLRenderer::Properties::HWAccelerated
        };

    public:
        explicit OpenGLRenderer(
            MainWindow& window,
            OpenGLRenderer::Properties flags = OpenGLRenderer::DefaultProperties);

        ~OpenGLRenderer() = default;

        SDL3::SDL_GLContext gl_context() const;

        bool clear();
        bool swap_buffers(MainWindow& window);

        ds::dims<i32> get_output_size() const;
        ds::rect<i32> get_viewport() const;

        bool set_viewport(const ds::rect<i32>& rect);
        bool set_draw_color(ds::color<f32> c);
        bool set_target();
        bool set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode);

    private:
        explicit OpenGLRenderer() = delete;
        explicit OpenGLRenderer(const OpenGLRenderer& other) = delete;
        explicit OpenGLRenderer(OpenGLRenderer& other) = delete;

    private:
        constexpr static inline ds::color m_bg_color{ rl::Colors::Background };
        Properties m_properties{ OpenGLRenderer::DefaultProperties };
        SDL3::SDL_GLContext m_sdl_glcontext{ nullptr };
    };
}
