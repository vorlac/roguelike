#pragma once

#include <bitset>

#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "utils/sdl_defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

struct NVGLUframebuffer;

namespace rl {
    class MainWindow;

    class OpenGLRenderer {
    public:
        struct BlendMode {
            using type = SDL3::SDL_BlendMode;
            constexpr static auto None = SDL_BLENDMODE_NONE;
            constexpr static auto Blend = SDL_BLENDMODE_BLEND;
            constexpr static auto Add = SDL_BLENDMODE_ADD;
            constexpr static auto Mod = SDL_BLENDMODE_MOD;
            constexpr static auto Mul = SDL_BLENDMODE_MUL;
            constexpr static auto Invalid = SDL_BLENDMODE_INVALID;
        };

    public:
        explicit OpenGLRenderer() = delete;
        explicit OpenGLRenderer(const OpenGLRenderer& other) = delete;
        explicit OpenGLRenderer(OpenGLRenderer& other) = delete;
        OpenGLRenderer& operator=(const OpenGLRenderer& other) = delete;
        OpenGLRenderer& operator=(OpenGLRenderer& other) = delete;

    public:
        explicit OpenGLRenderer(
            MainWindow& window);

        [[nodiscard]] SDL3::SDL_GLContext gl_context() const;
        [[nodiscard]] ds::dims<i32> get_output_size() const;
        [[nodiscard]] ds::rect<f32> get_viewport() const;

        bool clear() const;
        bool swap_buffers(const MainWindow& window) const;
        bool set_viewport(const ds::rect<i32>& rect) const;
        bool set_draw_color(ds::color<f32> c) const;
        bool set_target() const;
        bool set_draw_blend_mode(SDL3::SDL_BlendMode blend_mode) const;

    private:
        // TODO: move to style configs
        constexpr static ds::color m_bg_color{ rl::Colors::Background };
        SDL3::SDL_GLContext m_sdl_glcontext{ nullptr };
    };
}
