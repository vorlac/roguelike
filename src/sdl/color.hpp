#pragma once

#include <array>
#include <concepts>
#include <tuple>

#include <fmt/color.h>

#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/concepts.hpp"
#include "core/utils/conversions.hpp"
#include "core/utils/memory.hpp"
#include "sdl/defs.hpp"
SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
SDL_C_LIB_END

namespace rl::sdl {
    namespace internal {
        // style = fmt::fg(fmt::color::burly_wood);
        // fmt::print(style, fmt::format("color = dark_cyan\n"));
        //
        // fmt::fg(fmt::color::burly_wood)
        // fmt::fg(fmt::color::cornflower_blue);
        // fmt::fg(fmt::color::crimson);
        // fmt::fg(fmt::color::rosy_brown);
        // fmt::fg(fmt::color::gainsboro);
        // fmt::fg(fmt::color::tan);
        // fmt::fg(fmt::color::dark_golden_rod);
        // fmt::fg(fmt::color::dark_green);
        // fmt::fg(fmt::color::dark_olive_green);
        // fmt::fg(fmt::color::dark_sea_green);
        // fmt::fg(fmt::color::teal);
        // fmt::fg(fmt::color::light_gray);
        // fmt::fg(fmt::color::light_slate_gray);
        // fmt::fg(fmt::color::light_steel_blue);
        // fmt::fg(fmt::color::midnight_blue);
        // fmt::fg(fmt::color::silver);
        // fmt::fg(fmt::color::steel_blue);
        // fmt::fg(fmt::color::thistle);
        // fmt::fg(fmt::color::lavender);
    }

    struct color
    {
        enum Alpha : u8 {
            Transparent = 0,
            Opaque = 255,
        };

        constexpr inline color() = default;

        template <rl::integer T>
            requires(!std::same_as<T, u8>)
        constexpr inline color(T cr, T cg, T cb, T ca = Alpha::Opaque)
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

        constexpr inline color(u8 cr, u8 cg, u8 cb, u8 ca = Alpha::Opaque)
            : r{ cr }
            , g{ cg }
            , b{ cb }
            , a{ ca }
        {
        }

        // 0xRRGGBBAA
        constexpr inline color(u32 rgba)
            : r{ cast::to<u8>(0xff & (rgba >> (8 * 3))) }
            , g{ cast::to<u8>(0xff & (rgba >> (8 * 2))) }
            , b{ cast::to<u8>(0xff & (rgba >> (8 * 1))) }
            , a{ cast::to<u8>(0xff & (rgba >> (8 * 0))) }
        {
        }

        // 0x00RRGGBB
        constexpr inline color(fmt::rgb rgb)
            : r{ rgb.r }
            , g{ rgb.g }
            , b{ rgb.b }
            , a{ Alpha::Opaque }
        {
        }

        constexpr inline color(const SDL3::SDL_Color& c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr inline color(SDL3::SDL_Color&& c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr inline sdl::color operator+(const sdl::color& other) const
        {
            return {
                static_cast<u8>(r + other.r),
                static_cast<u8>(g + other.g),
                static_cast<u8>(b + other.b),
                static_cast<u8>(a + other.a),
            };
        }

        constexpr inline sdl::color operator-(const sdl::color& other) const
        {
            return {
                static_cast<u8>(r - other.r),
                static_cast<u8>(g - other.g),
                static_cast<u8>(b - other.b),
                static_cast<u8>(a - other.a),
            };
        }

        constexpr inline sdl::color operator*(const sdl::color& other) const
        {
            return {
                static_cast<u8>(r * other.r),
                static_cast<u8>(g * other.g),
                static_cast<u8>(b * other.b),
                static_cast<u8>(a * other.a),
            };
        }

        constexpr inline sdl::color operator/(const sdl::color& other) const
        {
            return {
                static_cast<u8>(r / other.r),
                static_cast<u8>(g / other.g),
                static_cast<u8>(b / other.b),
                static_cast<u8>(a / other.a),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::color operator+(const T val) const
        {
            return {
                static_cast<u8>(r + val),
                static_cast<u8>(g + val),
                static_cast<u8>(b + val),
                static_cast<u8>(a + val),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::color operator-(const T val) const
        {
            return {
                static_cast<u8>(r - val),
                static_cast<u8>(g - val),
                static_cast<u8>(b - val),
                static_cast<u8>(a - val),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::color operator*(const T val) const
        {
            return {
                static_cast<u8>(r * val),
                static_cast<u8>(g * val),
                static_cast<u8>(b * val),
                static_cast<u8>(a * val),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::color operator/(const T val) const
        {
            return {
                static_cast<u8>(r / val),
                static_cast<u8>(g / val),
                static_cast<u8>(b / val),
                static_cast<u8>(a / val),
            };
        }

        constexpr inline color& operator+=(const sdl::color& other)
        {
            *this = (*this + other);
            return *this;
        }

        constexpr inline color& operator-=(const sdl::color& other)
        {
            *this = (*this - other);
            return *this;
        }

        constexpr inline color& operator*=(const sdl::color& other)
        {
            *this = (*this * other);
            return *this;
        }

        constexpr inline color& operator/=(const sdl::color& other)
        {
            *this = (*this / other);
            return *this;
        }

    public:
        constexpr static inline color lerp(const color& s, const color& e, u8 w)
        {
            return color{ s + (e - s) * w };
        }

        inline u32 rgb(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGB(format, this->r, this->g, this->b);
        }

        inline u32 rgba(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGBA(format, this->r, this->g, this->b, this->a);
        }

        constexpr inline bool operator==(const color& other) const
        {
            return 0 == memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        constexpr inline bool is_empty() const
        {
            return this->operator==({});
        }

        constexpr inline bool operator!=(const color& other) const
        {
            return !this->operator==(other);
        }

        constexpr inline operator SDL3::SDL_Color()
        {
            return *((SDL3::SDL_Color*)this);
        }

        constexpr inline operator fmt::rgb() const
        {
            return fmt::rgb{ r, g, b };
        }

        constexpr inline operator fmt::text_style() const
        {
            return fmt::fg(static_cast<fmt::rgb>(*this));
        }

        constexpr inline operator std::array<u8, 4>() const
        {
            return std::array<u8, 4>{ r, g, b, a };
        }

        constexpr inline operator std::tuple<u8, u8, u8, u8>() const
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
