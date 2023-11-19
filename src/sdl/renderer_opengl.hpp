#pragma once

#include <glad/glad.h>

#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
SDL_C_LIB_END

namespace rl::sdl {
    class RendererGL
    {
    public:
        RendererGL()
        {
            SDL3::SDL_GL_LoadLibrary();
        }

        void test()
        {
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

            SDL_Window* window = SDL_CreateWindow("[glad] GL with SDL", SDL_WINDOWPOS_CENTERED,
                                                  SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                                  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

            SDL_GLContext context = SDL_GL_CreateContext(window);

            int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
            printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

            int exit = 0;
            while (!exit)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    switch (event.type)
                    {
                        case SDL_QUIT:
                            exit = 1;
                            break;
                        case SDL_KEYUP:
                            if (event.key.keysym.sym == SDLK_ESCAPE)
                                exit = 1;
                            break;
                        default:
                            break;
                    }
                }

                glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                SDL_GL_SwapWindow(window);
                SDL_Delay(1);
            }
        }

    private:
        SDL3::SDL_GLContext* m_sdl_glcontext{ nullptr };
    };
}
