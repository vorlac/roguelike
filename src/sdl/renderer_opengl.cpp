#include <glad/gl.h>

#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include <fmt/format.h>

#include "ds/dims.hpp"
#include "gl/shader.hpp"
#include "sdl/color.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer_opengl.hpp"
#include "sdl/window.hpp"
#include "utils/assert.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    RendererGL::RendererGL(const sdl::Window& window, RendererGL::Properties flags)
        : m_properties{ flags }
        , m_sdl_glcontext{ SDL3::SDL_GL_CreateContext(window.sdl_handle()) }
    {
        int result = m_sdl_glcontext != nullptr ? 0 : -1;
        sdl_assert(result == 0, "Failed to crete OpenGL context");

        result = SDL3::SDL_GL_LoadLibrary(nullptr);
        sdl_assert(result == 0, "Failed to load OpenGL library");
        if (result == 0)
        {
            sdl_assert(m_sdl_glcontext != nullptr, "failed to create renderer");

            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_DOUBLEBUFFER, 1);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
            SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_PROFILE_MASK,
                                      SDL3::SDL_GL_CONTEXT_PROFILE_CORE);

            i32 version = gladLoadGL(SDL3::SDL_GL_GetProcAddress);
            i32 gl_major_ver{ GLAD_VERSION_MAJOR(version) };
            i32 gl_minor_ver{ GLAD_VERSION_MINOR(version) };
            log::warning("OpenGL [{}.{}] => Context Created Successfully", gl_major_ver,
                         gl_minor_ver);

            runtime_assert((gl_major_ver >= 3 && gl_minor_ver >= 3 || gl_major_ver > 3),
                           "Deprecated OpenGL Version Loaded: {}.{}", gl_major_ver, gl_minor_ver);

            if (gl_major_ver >= 3 && gl_minor_ver >= 3 || gl_major_ver > 3)
            {
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

    bool RendererGL::clear(const sdl::Color& c)
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

    bool RendererGL::set_draw_color(const sdl::Color& c)
    {
        i32 result = 0;
        return result == 0;
    }

    bool RendererGL::draw_texture(sdl::Texture& texture, const ds::rect<f32>& src_rect,
                                  const ds::rect<f32>& dst_rect)
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

    bool RendererGL::draw_point(const ds::point<f32>& pt)
    {
        i32 result = 0;
        return result == 0;
    }

    ds::rect<i32> RendererGL::get_viewport()
    {
        ds::rect<i32> rect{ 0, 0, 0, 0 };
        return rect;
    }

    bool RendererGL::draw_points(const std::vector<ds::point<f32>>& points)
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::draw_line(const ds::point<f32>& pt1, const ds::point<f32>& pt2)
    {
        i32 result = 0;
        return result == 0;
    }

    bool RendererGL::draw_lines(const std::vector<ds::point<f32>>& lines)
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::draw_triangle(const ds::triangle<f32>& triangle, const sdl::Color& color)
    {
        i32 result = 0;
        return result == 0;
    }

    bool RendererGL::draw_rect(ds::rect<f32>&& rect, const sdl::Color& c)
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::draw_rects(const std::vector<ds::rect<f32>>& rects)
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::fill_rect(const ds::rect<f32>& rect, const sdl::Color& c)
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::fill_rects(const std::vector<ds::rect<f32>>& rects, const sdl::Color& c)
    {
        i32 result = 0;

        return result == 0;
    }

    bool RendererGL::fill_rects(const std::vector<std::pair<ds::rect<f32>, sdl::Color>>& rects)
    {
        bool ret = true;

        return ret;
    }
}
