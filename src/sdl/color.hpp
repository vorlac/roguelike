#pragma once

#include <array>

#include "core/numeric_types.hpp"

namespace SDL3
{
#include <SDL3/SDL.h>
}

namespace rl::sdl
{
    struct color
    {
        u8 r{ 0 };
        u8 g{ 0 };
        u8 b{ 0 };
        u8 a{ 0 };

        constexpr color() = default;

        constexpr color(u8 cr, u8 cg, u8 cb, u8 ca)
            : r{ cr }
            , g{ cg }
            , b{ cb }
            , a{ ca }
        {
        }

        constexpr color(const SDL3::SDL_Color& c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr color(SDL3::SDL_Color c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        operator SDL3::SDL_Color()
        {
            return *reinterpret_cast<SDL3::SDL_Color*>(this);
        }

        constexpr operator std::array<u8, 4>()
        {
            return std::array<u8, 4>{ r, g, b, a };
        }

        constexpr operator std::tuple<u8, u8, u8, u8>()
        {
            return { r, g, b, a };
        }
    };
}
