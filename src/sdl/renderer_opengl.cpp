#include <string>
#include <string_view>

#include <fmt/format.h>
#include <glad/gl.h>

#include "ds/dimensions.hpp"
#include "sdl/defs.hpp"
#include "sdl/renderer_opengl.hpp"
#include "sdl/window.hpp"
#include "utils/assert.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {

    static std::string get_opengl_version()
    {
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_DOUBLEBUFFER, 1);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL3::SDL_GL_SetAttribute(SDL3::SDL_GL_CONTEXT_PROFILE_MASK,
                                  SDL3::SDL_GL_CONTEXT_PROFILE_CORE);
        int version = gladLoadGL((GLADloadfunc)SDL3::SDL_GL_GetProcAddress);
        return fmt::format("{}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    }

    RendererGL::RendererGL(const sdl::Window& window, std::string_view driver,
                           RendererGL::Properties flags)
        : m_properties{ flags }
        , m_sdl_glcontext{ SDL3::SDL_GL_CreateContext(window.sdl_handle()) }
    {
        int result = m_sdl_glcontext != nullptr ? 0 : -1;
        sdl_assert(result == 0, "Failed to crete OpenGL context");

        // result = SDL3::SDL_GL_LoadLibrary(nullptr);
        // sdl_assert(result == 0, "Failed to load OpenGL library");
        if (result == 0)
        {
            sdl_assert(m_sdl_glcontext != nullptr, "failed to create renderer");

            log::info("Successfully Loaded OpenGL {}", get_opengl_version());

            ds::dims<i32> viewport{ window.get_render_size() };
            glViewport(0, 0, viewport.width, viewport.height);
        }
    }
}
