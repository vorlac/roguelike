#pragma once

#include <concepts>

#include "core/numeric.hpp"
#include "sdl/defs.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"
#include "utils/assert.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
SDL_C_LIB_END

namespace rl::sdl {
    template <typename T>
    class scoped_lock
    {
    public:
        scoped_lock(sdl::Surface& sur)
            requires std::same_as<T, sdl::Surface>
            : m_sdl_lockable{ sur }
        {
            if (SDL_MUSTLOCK(m_sdl_lockable.sdl_handle()))
            {
                i32 result = SDL3::SDL_LockSurface(m_sdl_lockable.sdl_handle());
                runtime_assert(result == 0, "failed to lock surface");
            }
            m_sdl_lockable.is_locked = true;
        }

        scoped_lock(sdl::Texture& tex)
            requires std::same_as<T, sdl::Texture>
            : m_sdl_lockable{ tex }
        {
            SDL3::SDL_Texture* sdl_texture{ m_sdl_lockable.sdl_handle() };

            if (SDL_MUSTLOCK(m_sdl_lockable.sdl_handle()))
            {
                i32 pitch{ sdl_texture->pitch };
                void* pixels{ sdl_texture->pixels };
                i32 result = SDL3::SDL_LockTexture(sdl_texture, nullptr, &pixels, &pitch);
                runtime_assert(result == 0, "failed to lock surface");
            }
        }

        ~scoped_lock()
            requires std::same_as<T, sdl::Surface>
        {
            SDL3::SDL_UnlockSurface(m_sdl_lockable.sdl_handle());
            m_sdl_lockable.is_locked = false;
        }

    private:
        // must pass in data
        scoped_lock() = delete;
        // can't lock immutable data
        scoped_lock(const T&) = delete;

    private:
        T& m_sdl_lockable{};
    };
}
