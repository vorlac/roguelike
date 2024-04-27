#include <glad/gl.h>

#include <memory>
#include <tuple>

#include <fmt/format.h>

#include "core/assert.hpp"
#include "core/main_window.hpp"
#include "core/renderer.hpp"
#include "gfx/nvg_renderer.hpp"
#include "sdl/defs.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"

SDL_C_LIB_BEGIN
struct SDL_VideoDevice;
SDL_C_LIB_END

namespace rl {
    namespace detail {
        static SDL3::SDL_GLContext create_opengl_context(SDL3::SDL_Window* sdl_window)
        {
            sdl_assert(sdl_window != nullptr,
                       "Attempting to create context from uninitialized window");
            const SDL3::SDL_GLContext gl_context{ SDL3::SDL_GL_CreateContext(sdl_window) };
            debug_assert(gl_context != nullptr, "Failed to create OpenGL context");

            const i32 gl_version = gladLoadGL(SDL3::SDL_GL_GetProcAddress);
            i32 gl_major_ver{ GLAD_VERSION_MAJOR(gl_version) };
            i32 gl_minor_ver{ GLAD_VERSION_MINOR(gl_version) };

            debug_assert((gl_major_ver >= 3 && gl_minor_ver >= 3 || gl_major_ver > 3),
                         "Deprecated OpenGL Version Loaded: {}.{}", gl_major_ver, gl_minor_ver);

            if (gl_major_ver > 3 || (gl_major_ver == 3 && gl_minor_ver >= 3)) {
                const GLubyte* const gl_ver_str = glGetString(GL_VERSION);
                const GLubyte* const renderer_str = glGetString(GL_RENDERER);
                log::warning("GL_RENDERER = {}", reinterpret_cast<const char*>(renderer_str));
                log::warning("GL_VERSION = {}", reinterpret_cast<const char*>(gl_ver_str));
                log::warning("OpenGL [{}.{}] Context Created Successfully", gl_major_ver, gl_minor_ver);
            }

            return gl_context;
        }
    }

    OpenGLRenderer::OpenGLRenderer(MainWindow& window, const OpenGLRenderer::Properties flags)
        : m_properties{ flags }
        , m_sdl_glcontext{ detail::create_opengl_context(window.sdl_handle()) }
    {
        if (m_sdl_glcontext != nullptr) {
            sdl_assert(m_sdl_glcontext != nullptr, "failed to create renderer");
            const ds::dims<i32> viewport{ window.get_render_size() };
            glViewport(0, 0, viewport.width, viewport.height);
        }
    }

    bool OpenGLRenderer::clear() const
    {
        glClearColor(m_bg_color.r, m_bg_color.g, m_bg_color.b, m_bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        return true;
    }

    bool OpenGLRenderer::swap_buffers(const rl::MainWindow& window) const
    {
        const i32 result{ SDL3::SDL_GL_SwapWindow(window.sdl_handle()) };
        sdl_assert(result == 0, "OpenGL renderer buffer swap failed");
        return result == 0;
    }

    SDL3::SDL_GLContext OpenGLRenderer::gl_context() const
    {
        return m_sdl_glcontext;
    }

    ds::dims<i32> OpenGLRenderer::get_output_size() const
    {
        debug_assert("not implemented");
        return ds::dims<i32>{ 0, 0 };
    }

    bool OpenGLRenderer::set_draw_color(ds::color<f32>) const
    {
        constexpr i32 result{ 0 };
        debug_assert("not implemented");
        return result == 0;
    }

    bool OpenGLRenderer::set_target() const
    {
        constexpr i32 result{ 0 };
        debug_assert("not implemented");
        return result == 0;
    }

    bool OpenGLRenderer::set_draw_blend_mode(const SDL3::SDL_BlendMode) const
    {
        constexpr i32 result{ 0 };
        debug_assert("not implemented");
        return result == 0;
    }

    ds::rect<f32> OpenGLRenderer::get_viewport() const
    {
        std::array buff{ 0, 0, 0, 0 };
        glGetIntegerv(GL_VIEWPORT, buff.data());
        debug_assert(buff[2] > 0 && buff[3] > 0, "failed to get viewport");

        return ds::rect<f32>{
            ds::point<f32>{
                static_cast<f32>(buff[0]),
                static_cast<f32>(buff[1]),
            },
            ds::dims<f32>{
                static_cast<f32>(buff[2]),
                static_cast<f32>(buff[3]),
            },
        };
    }

    bool OpenGLRenderer::set_viewport(const ds::rect<i32>& rect) const
    {
        debug_assert(!rect.is_empty(), "invalid viewport rect being set");
        glViewport(rect.pt.x, rect.pt.y, rect.size.width, rect.size.height);
        return !rect.is_empty();
    }
}
