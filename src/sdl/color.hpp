#pragma once

#include <array>
#include <concepts>
#include <tuple>

#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/concepts.hpp"
#include "core/utils/conversions.hpp"
#include "core/utils/memory.hpp"

namespace SDL3
{
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
}

namespace rl::sdl
{
    struct color
    {
        enum Alpha : u8 {
            Transparent = 0,
            Opaque = 255,
        };

        inline static constexpr u8 ALPHA_OPAQUE = 255;
        inline static constexpr u8 ALPHA_TRANSPARENT = 0;

        inline constexpr color() = default;

        template <rl::integer T>
            requires(!std::same_as<T, u8>)
        inline constexpr color(T cr, T cg, T cb, T ca = Alpha::Opaque)
            : r{ cast::to<u8>(cr) }
            , g{ cast::to<u8>(cg) }
            , b{ cast::to<u8>(cb) }
            , a{ cast::to<u8>(ca) }
        {
            runtime_assert(
                (cr <= std::numeric_limits<u8>::max() && cg <= std::numeric_limits<u8>::max() &&
                 cb <= std::numeric_limits<u8>::max() && ca <= std::numeric_limits<u8>::max()),
                "overflow representing r,g,b,a color components");
        }

        inline constexpr color(u8 cr, u8 cg, u8 cb, u8 ca = Alpha::Opaque)
            : r{ cr }
            , g{ cg }
            , b{ cb }
            , a{ ca }
        {
        }

        inline constexpr color(const SDL3::SDL_Color& c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        inline constexpr color(SDL3::SDL_Color c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        inline u32 rgb(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGB(format, this->r, this->g, this->b);
        }

        inline u32 rgba(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGBA(format, this->r, this->g, this->b, this->a);
        }

        inline constexpr bool operator==(const sdl::color& other)
        {
            return 0 == memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        inline constexpr bool operator!=(const sdl::color& other)
        {
            return this->operator==(other);
        }

        inline operator SDL3::SDL_Color()
        {
            return *reinterpret_cast<SDL3::SDL_Color*>(this);
        }

        inline constexpr operator std::array<u8, 4>()
        {
            return std::array<u8, 4>{ r, g, b, a };
        }

        inline constexpr operator std::tuple<u8, u8, u8, u8>()
        {
            return { r, g, b, a };
        }

    public:
        u8 r{ 0 };
        u8 g{ 0 };
        u8 b{ 0 };
        u8 a{ Alpha::Opaque };
    };
}
