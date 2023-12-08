#include <glad/gl.h>

#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include <fmt/format.h>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "gl/shader.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer_opengl.hpp"
#include "sdl/window.hpp"
#include "utils/assert.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
struct SDL_VideoDevice;
typedef struct SDL_VideoDevice SDL_VideoDevice;
SDL_C_LIB_END

namespace rl::sdl {
    static inline SDL3::SDL_GLContext create_opengl_context(SDL3::SDL_Window* sdl_window)
    {
        sdl_assert(sdl_window != nullptr, "Creating OpenGL context from NULL window");
        // SDL3::SDL_VideoDevice* device{ SDL3::SDL_GetVideoDevice() };
        return SDL3::SDL_GL_CreateContext(sdl_window);
    }

    RendererGL::RendererGL(sdl::Window& window, RendererGL::Properties flags)
        : m_properties{ flags }
        , m_sdl_glcontext{ create_opengl_context(window.sdl_handle()) }
    {
        // wglCreateContext();
        int result = m_sdl_glcontext != nullptr ? 0 : -1;
        sdl_assert(result == 0, "Failed to create OpenGL context");

        i32 version = gladLoadGL((GLADloadfunc)SDL3::SDL_GL_GetProcAddress);
        i32 gl_major_ver{ GLAD_VERSION_MAJOR(version) };
        i32 gl_minor_ver{ GLAD_VERSION_MINOR(version) };

        runtime_assert((gl_major_ver >= 3 && gl_minor_ver >= 3 || gl_major_ver > 3),
                       "Deprecated OpenGL Version Loaded: {}.{}", gl_major_ver, gl_minor_ver);

        if (gl_major_ver > 3 || (gl_major_ver == 3 && gl_minor_ver >= 3))
        {
            const GLubyte* const gl_ver_str = glGetString(GL_VERSION);
            const GLubyte* const renderer_str = glGetString(GL_RENDERER);
            log::warning("GL_RENDERER = {}", reinterpret_cast<const char*>(renderer_str));
            log::warning("GL_VERSION = {}", reinterpret_cast<const char*>(gl_ver_str));
            log::warning("OpenGL [{}.{}] Context Created Successfully", gl_major_ver, gl_minor_ver);

            if (result == 0)
            {
                sdl_assert(m_sdl_glcontext != nullptr, "failed to create renderer");
                ds::dims<i32> viewport{ window.get_render_size() };
                glViewport(0, 0, viewport.width, viewport.height);
            }
        }
    }

    bool RendererGL::present()
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::swap_buffers(sdl::Window& window)
    {
        i32 result = SDL3::SDL_GL_SwapWindow(window.sdl_handle());
        sdl_assert(result == 0, "OpenGL renderer buffer swap failed");
        return result == 0;
    }

    bool RendererGL::clear(const ds::color<u8>& c)
    {
        bool ret = true;
        std::tuple<f32, f32, f32, f32> clr{ c };
        glClearColor(std::get<0>(clr), std::get<1>(clr), std::get<2>(clr), std::get<3>(clr));
        glClear(GL_COLOR_BUFFER_BIT);
        return ret;
    }

    ds::dims<i32> RendererGL::get_output_size() const
    {
        ds::dims<i32> s{ 0, 0 };

        return s;
    }

    bool RendererGL::set_draw_color(const ds::color<u8>& c)
    {
        i32 result = 0;
        return result == 0;
    }

    bool RendererGL::set_target()
    {
        i32 result = 0;
        return result == 0;
    }

    bool RendererGL::set_target(sdl::Texture& tex)
    {
        i32 result = 0;
        return result == 0;
    }

    bool RendererGL::set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode)
    {
        i32 result = 0;
        return result == 0;
    }

    ds::rect<i32> RendererGL::get_viewport() const
    {
        std::array<i32, 4> buff{ 0, 0, 0, 0 };
        glGetIntegerv(GL_VIEWPORT, reinterpret_cast<i32*>(buff.data()));
        ds::rect<i32> rect{ { buff[0], buff[1] }, { buff[2], buff[3] } };
        sdl_assert(!rect.is_empty(), "failed to get viewport");
        return rect;
    }

    bool RendererGL::set_viewport(const ds::rect<i32>& rect)
    {
        runtime_assert(!rect.is_empty(), "invalid viewport rect being set");
        glViewport(rect.pt.x, rect.pt.y, rect.size.width, rect.size.height);
        return !rect.is_empty();
    }
}
