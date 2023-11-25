#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/color.h>

#include "core/math.hpp"
#include "core/numeric_types.hpp"
#include "sdl/defs.hpp"
#include "utils/assert.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/memory.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
SDL_C_LIB_END

namespace rl::sdl {
    struct Color
    {
    public:
        struct Preset
        {
            enum alpha : u8 {
                Transparent = 0,
                Opaque = 255,
            };

            constexpr static inline std::tuple alice_blue{ 0xF0, 0xF8, 0xFF };
            constexpr static inline std::tuple antique_white{ 0xFA, 0xEB, 0xD7 };
            constexpr static inline std::tuple aqua{ 0x00, 0xFF, 0xFF };
            constexpr static inline std::tuple aquamarine{ 0x7F, 0xFF, 0xD4 };
            constexpr static inline std::tuple azure{ 0xF0, 0xFF, 0xFF };
            constexpr static inline std::tuple beige{ 0xF5, 0xF5, 0xDC };
            constexpr static inline std::tuple bisque{ 0xFF, 0xE4, 0xC4 };
            constexpr static inline std::tuple black{ 0x00, 0x00, 0x00 };
            constexpr static inline std::tuple blanched_almond{ 0xFF, 0xEB, 0xCD };
            constexpr static inline std::tuple blue{ 0x00, 0x00, 0xFF };
            constexpr static inline std::tuple blue_violet{ 0x8A, 0x2B, 0xE2 };
            constexpr static inline std::tuple Brown{ 0xA5, 0x2A, 0x2A };
            constexpr static inline std::tuple burly_wood{ 0xDE, 0xB8, 0x87 };
            constexpr static inline std::tuple cadet_blue{ 0x5F, 0x9E, 0xA0 };
            constexpr static inline std::tuple chartreuse{ 0x7F, 0xFF, 0x00 };
            constexpr static inline std::tuple chocolate{ 0xD2, 0x69, 0x1E };
            constexpr static inline std::tuple coral{ 0xFF, 0x7F, 0x50 };
            constexpr static inline std::tuple cornflower_blue{ 0x64, 0x95, 0xED };
            constexpr static inline std::tuple cornsilk{ 0xFF, 0xF8, 0xDC };
            constexpr static inline std::tuple crimson{ 0xDC, 0x14, 0x3C };
            constexpr static inline std::tuple cyan{ 0x00, 0xFF, 0xFF };
            constexpr static inline std::tuple dark_blue{ 0x00, 0x00, 0x8B };
            constexpr static inline std::tuple dark_cyan{ 0x00, 0x8B, 0x8B };
            constexpr static inline std::tuple dark_golden_rod{ 0xB8, 0x86, 0x0B };
            constexpr static inline std::tuple dark_gray{ 0xA9, 0xA9, 0xA9 };
            constexpr static inline std::tuple dark_grey{ 0xA9, 0xA9, 0xA9 };
            constexpr static inline std::tuple dark_green{ 0x00, 0x64, 0x00 };
            constexpr static inline std::tuple dark_khaki{ 0xBD, 0xB7, 0x6B };
            constexpr static inline std::tuple dark_magenta{ 0x8B, 0x00, 0x8B };
            constexpr static inline std::tuple dark_olive_green{ 0x55, 0x6B, 0x2F };
            constexpr static inline std::tuple dark_orange{ 0xFF, 0x8C, 0x00 };
            constexpr static inline std::tuple dark_orchid{ 0x99, 0x32, 0xCC };
            constexpr static inline std::tuple dark_red{ 0x8B, 0x00, 0x00 };
            constexpr static inline std::tuple dark_salmon{ 0xE9, 0x96, 0x7A };
            constexpr static inline std::tuple dark_sea_green{ 0x8F, 0xBC, 0x8F };
            constexpr static inline std::tuple dark_slate_blue{ 0x48, 0x3D, 0x8B };
            constexpr static inline std::tuple dark_slate_gray{ 0x2F, 0x4F, 0x4F };
            constexpr static inline std::tuple dark_slate_grey{ 0x2F, 0x4F, 0x4F };
            constexpr static inline std::tuple dark_turquoise{ 0x00, 0xCE, 0xD1 };
            constexpr static inline std::tuple dark_violet{ 0x94, 0x00, 0xD3 };
            constexpr static inline std::tuple deep_pink{ 0xFF, 0x14, 0x93 };
            constexpr static inline std::tuple deep_sky_blue{ 0x00, 0xBF, 0xFF };
            constexpr static inline std::tuple dim_gray{ 0x69, 0x69, 0x69 };
            constexpr static inline std::tuple dim_grey{ 0x69, 0x69, 0x69 };
            constexpr static inline std::tuple dodger_blue{ 0x1E, 0x90, 0xFF };
            constexpr static inline std::tuple fire_brick{ 0xB2, 0x22, 0x22 };
            constexpr static inline std::tuple floral_white{ 0xFF, 0xFA, 0xF0 };
            constexpr static inline std::tuple forest_green{ 0x22, 0x8B, 0x22 };
            constexpr static inline std::tuple Fuchsia{ 0xFF, 0x00, 0xFF };
            constexpr static inline std::tuple gainsboro{ 0xDC, 0xDC, 0xDC };
            constexpr static inline std::tuple ghost_white{ 0xF8, 0xF8, 0xFF };
            constexpr static inline std::tuple gold{ 0xFF, 0xD7, 0x00 };
            constexpr static inline std::tuple golden_rod{ 0xDA, 0xA5, 0x20 };
            constexpr static inline std::tuple gray{ 0x80, 0x80, 0x80 };
            constexpr static inline std::tuple grey{ 0x80, 0x80, 0x80 };
            constexpr static inline std::tuple green{ 0x00, 0x80, 0x00 };
            constexpr static inline std::tuple green_yellow{ 0xAD, 0xFF, 0x2F };
            constexpr static inline std::tuple honey_dew{ 0xF0, 0xFF, 0xF0 };
            constexpr static inline std::tuple hot_pink{ 0xFF, 0x69, 0xB4 };
            constexpr static inline std::tuple indian_red{ 0xCD, 0x5C, 0x5C };
            constexpr static inline std::tuple indigo{ 0x4B, 0x00, 0x82 };
            constexpr static inline std::tuple ivory{ 0xFF, 0xFF, 0xF0 };
            constexpr static inline std::tuple khaki{ 0xF0, 0xE6, 0x8C };
            constexpr static inline std::tuple lavender{ 0xE6, 0xE6, 0xFA };
            constexpr static inline std::tuple lavender_blush{ 0xFF, 0xF0, 0xF5 };
            constexpr static inline std::tuple lawn_green{ 0x7C, 0xFC, 0x00 };
            constexpr static inline std::tuple lemon_chiffon{ 0xFF, 0xFA, 0xCD };
            constexpr static inline std::tuple light_blue{ 0xAD, 0xD8, 0xE6 };
            constexpr static inline std::tuple light_coral{ 0xF0, 0x80, 0x80 };
            constexpr static inline std::tuple light_cyan{ 0xE0, 0xFF, 0xFF };
            constexpr static inline std::tuple light_golden_rod_yellow{ 0xFA, 0xFA, 0xD2 };
            constexpr static inline std::tuple light_gray{ 0xD3, 0xD3, 0xD3 };
            constexpr static inline std::tuple light_grey{ 0xD3, 0xD3, 0xD3 };
            constexpr static inline std::tuple light_green{ 0x90, 0xEE, 0x90 };
            constexpr static inline std::tuple light_pink{ 0xFF, 0xB6, 0xC1 };
            constexpr static inline std::tuple light_salmon{ 0xFF, 0xA0, 0x7A };
            constexpr static inline std::tuple light_sea_green{ 0x20, 0xB2, 0xAA };
            constexpr static inline std::tuple light_sky_blue{ 0x87, 0xCE, 0xFA };
            constexpr static inline std::tuple light_slate_gray{ 0x77, 0x88, 0x99 };
            constexpr static inline std::tuple light_slate_grey{ 0x77, 0x88, 0x99 };
            constexpr static inline std::tuple light_steel_blue{ 0xB0, 0xC4, 0xDE };
            constexpr static inline std::tuple light_yellow{ 0xFF, 0xFF, 0xE0 };
            constexpr static inline std::tuple Lime{ 0x00, 0xFF, 0x00 };
            constexpr static inline std::tuple lime_green{ 0x32, 0xCD, 0x32 };
            constexpr static inline std::tuple linen{ 0xFA, 0xF0, 0xE6 };
            constexpr static inline std::tuple magenta{ 0xFF, 0x00, 0xFF };
            constexpr static inline std::tuple maroon{ 0x80, 0x00, 0x00 };
            constexpr static inline std::tuple medium_aqua_marine{ 0x66, 0xCD, 0xAA };
            constexpr static inline std::tuple medium_blue{ 0x00, 0x00, 0xCD };
            constexpr static inline std::tuple medium_orchid{ 0xBA, 0x55, 0xD3 };
            constexpr static inline std::tuple medium_purple{ 0x93, 0x70, 0xDB };
            constexpr static inline std::tuple medium_sea_green{ 0x3C, 0xB3, 0x71 };
            constexpr static inline std::tuple medium_slate_blue{ 0x7B, 0x68, 0xEE };
            constexpr static inline std::tuple medium_spring_green{ 0x00, 0xFA, 0x9A };
            constexpr static inline std::tuple medium_turquoise{ 0x48, 0xD1, 0xCC };
            constexpr static inline std::tuple medium_violet_red{ 0xC7, 0x15, 0x85 };
            constexpr static inline std::tuple midnight_blue{ 0x19, 0x19, 0x70 };
            constexpr static inline std::tuple mint_cream{ 0xF5, 0xFF, 0xFA };
            constexpr static inline std::tuple misty_rose{ 0xFF, 0xE4, 0xE1 };
            constexpr static inline std::tuple moccasin{ 0xFF, 0xE4, 0xB5 };
            constexpr static inline std::tuple navajo_white{ 0xFF, 0xDE, 0xAD };
            constexpr static inline std::tuple navy{ 0x00, 0x00, 0x80 };
            constexpr static inline std::tuple old_lace{ 0xFD, 0xF5, 0xE6 };
            constexpr static inline std::tuple Olive{ 0x80, 0x80, 0x00 };
            constexpr static inline std::tuple olive_drab{ 0x6B, 0x8E, 0x23 };
            constexpr static inline std::tuple Orange{ 0xFF, 0xA5, 0x00 };
            constexpr static inline std::tuple orange_red{ 0xFF, 0x45, 0x00 };
            constexpr static inline std::tuple orchid{ 0xDA, 0x70, 0xD6 };
            constexpr static inline std::tuple pale_golden_rod{ 0xEE, 0xE8, 0xAA };
            constexpr static inline std::tuple pale_green{ 0x98, 0xFB, 0x98 };
            constexpr static inline std::tuple pale_turquoise{ 0xAF, 0xEE, 0xEE };
            constexpr static inline std::tuple pale_violet_red{ 0xDB, 0x70, 0x93 };
            constexpr static inline std::tuple papaya_whip{ 0xFF, 0xEF, 0xD5 };
            constexpr static inline std::tuple peach_puff{ 0xFF, 0xDA, 0xB9 };
            constexpr static inline std::tuple peru{ 0xCD, 0x85, 0x3F };
            constexpr static inline std::tuple pink{ 0xFF, 0xC0, 0xCB };
            constexpr static inline std::tuple plum{ 0xDD, 0xA0, 0xDD };
            constexpr static inline std::tuple powder_blue{ 0xB0, 0xE0, 0xE6 };
            constexpr static inline std::tuple purple{ 0x80, 0x00, 0x80 };
            constexpr static inline std::tuple rebecca_purple{ 0x66, 0x33, 0x99 };
            constexpr static inline std::tuple red{ 0xFF, 0x00, 0x00 };
            constexpr static inline std::tuple rosy_brown{ 0xBC, 0x8F, 0x8F };
            constexpr static inline std::tuple royal_blue{ 0x41, 0x69, 0xE1 };
            constexpr static inline std::tuple saddle_brown{ 0x8B, 0x45, 0x13 };
            constexpr static inline std::tuple salmon{ 0xFA, 0x80, 0x72 };
            constexpr static inline std::tuple sandy_brown{ 0xF4, 0xA4, 0x60 };
            constexpr static inline std::tuple sea_green{ 0x2E, 0x8B, 0x57 };
            constexpr static inline std::tuple sea_shell{ 0xFF, 0xF5, 0xEE };
            constexpr static inline std::tuple Sienna{ 0xA0, 0x52, 0x2D };
            constexpr static inline std::tuple Silver{ 0xC0, 0xC0, 0xC0 };
            constexpr static inline std::tuple sky_blue{ 0x87, 0xCE, 0xEB };
            constexpr static inline std::tuple slate_blue{ 0x6A, 0x5A, 0xCD };
            constexpr static inline std::tuple slate_gray{ 0x70, 0x80, 0x90 };
            constexpr static inline std::tuple slate_grey{ 0x70, 0x80, 0x90 };
            constexpr static inline std::tuple snow{ 0xFF, 0xFA, 0xFA };
            constexpr static inline std::tuple spring_green{ 0x00, 0xFF, 0x7F };
            constexpr static inline std::tuple steel_blue{ 0x46, 0x82, 0xB4 };
            constexpr static inline std::tuple tan{ 0xD2, 0xB4, 0x8C };
            constexpr static inline std::tuple teal{ 0x00, 0x80, 0x80 };
            constexpr static inline std::tuple thistle{ 0xD8, 0xBF, 0xD8 };
            constexpr static inline std::tuple tomato{ 0xFF, 0x63, 0x47 };
            constexpr static inline std::tuple turquoise{ 0x40, 0xE0, 0xD0 };
            constexpr static inline std::tuple violet{ 0xEE, 0x82, 0xEE };
            constexpr static inline std::tuple wheat{ 0xF5, 0xDE, 0xB3 };
            constexpr static inline std::tuple white{ 0xFF, 0xFF, 0xFF };
            constexpr static inline std::tuple white_smoke{ 0xF5, 0xF5, 0xF5 };
            constexpr static inline std::tuple yellow{ 0xFF, 0xFF, 0x00 };
            constexpr static inline std::tuple yellow_green{ 0x9A, 0xCD, 0x32 };
        };

    public:
        constexpr inline Color() = default;

        template <rl::integer T>
            requires(!std::same_as<T, u8>)
        constexpr inline Color(T cr, T cg, T cb, T ca = Preset::alpha::Opaque)
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

        template <rl::floating_point T>
            requires std::same_as<T, f32>
        constexpr inline Color(T cr, T cg, T cb, T ca = 1.0f)
            : r{ rl::math::clamp<u8>(cr * 255.0f, 0, 255) }
            , g{ rl::math::clamp<u8>(cg * 255.0f, 0, 255) }
            , b{ rl::math::clamp<u8>(cb * 255.0f, 0, 255) }
            , a{ rl::math::clamp<u8>(ca * 255.0f, 0, 255) }
        {
            runtime_assert((cr * 255.0f <= std::numeric_limits<u8>::max() &&
                            cg * 255.0f <= std::numeric_limits<u8>::max() &&
                            cb * 255.0f <= std::numeric_limits<u8>::max() &&
                            ca * 255.0f <= std::numeric_limits<u8>::max()),
                           "overflow representing r,g,b,a color components");
        }

        constexpr inline Color(std::tuple<u8, u8, u8> tup)
            : r{ std::get<0>(tup) }
            , g{ std::get<1>(tup) }
            , b{ std::get<2>(tup) }
            , a{ Preset::alpha::Opaque }
        {
        }

        template <rl::integer T>
        constexpr inline Color& operator=(std::tuple<T, T, T> tup)
        {
            r = cast::to<u8>(std::get<0>());
            g = cast::to<u8>(std::get<1>());
            b = cast::to<u8>(std::get<2>());
            return *this;
        }

        constexpr inline Color(u8 cr, u8 cg, u8 cb, u8 ca = Preset::Opaque)
            : r{ cr }
            , g{ cg }
            , b{ cb }
            , a{ ca }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr inline Color(u32 rgba)
            : r{ cast::to<u8>(0xff & (rgba >> (8 * 3))) }
            , g{ cast::to<u8>(0xff & (rgba >> (8 * 2))) }
            , b{ cast::to<u8>(0xff & (rgba >> (8 * 1))) }
            , a{ cast::to<u8>(0xff & (rgba >> (8 * 0))) }
        {
        }

        // 0x00RRGGBB
        constexpr inline Color(fmt::rgb rgb)
            : r{ rgb.r }
            , g{ rgb.g }
            , b{ rgb.b }
            , a{ Preset::alpha::Opaque }
        {
        }

        constexpr inline Color(const SDL3::SDL_Color& c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr inline Color(SDL3::SDL_Color&& c)
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr inline sdl::Color operator+(const sdl::Color& other) const
        {
            return {
                static_cast<u8>(r + other.r),
                static_cast<u8>(g + other.g),
                static_cast<u8>(b + other.b),
                static_cast<u8>(a + other.a),
            };
        }

        constexpr inline sdl::Color operator-(const sdl::Color& other) const
        {
            return {
                static_cast<u8>(r - other.r),
                static_cast<u8>(g - other.g),
                static_cast<u8>(b - other.b),
                static_cast<u8>(a - other.a),
            };
        }

        constexpr inline sdl::Color operator*(const sdl::Color& other) const
        {
            return {
                static_cast<u8>(r * other.r),
                static_cast<u8>(g * other.g),
                static_cast<u8>(b * other.b),
                static_cast<u8>(a * other.a),
            };
        }

        constexpr inline sdl::Color operator/(const sdl::Color& other) const
        {
            return {
                static_cast<u8>(r / other.r),
                static_cast<u8>(g / other.g),
                static_cast<u8>(b / other.b),
                static_cast<u8>(a / other.a),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::Color operator+(const T val) const
        {
            return {
                static_cast<u8>(r + val),
                static_cast<u8>(g + val),
                static_cast<u8>(b + val),
                static_cast<u8>(a + val),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::Color operator-(const T val) const
        {
            return {
                static_cast<u8>(r - val),
                static_cast<u8>(g - val),
                static_cast<u8>(b - val),
                static_cast<u8>(a - val),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::Color operator*(const T val) const
        {
            return {
                static_cast<u8>(r * val),
                static_cast<u8>(g * val),
                static_cast<u8>(b * val),
                static_cast<u8>(a * val),
            };
        }

        template <rl::numeric T>
        constexpr inline sdl::Color operator/(const T val) const
        {
            return {
                static_cast<u8>(r / val),
                static_cast<u8>(g / val),
                static_cast<u8>(b / val),
                static_cast<u8>(a / val),
            };
        }

        constexpr inline Color& operator+=(const sdl::Color& other)
        {
            *this = (*this + other);
            return *this;
        }

        constexpr inline Color& operator-=(const sdl::Color& other)
        {
            *this = (*this - other);
            return *this;
        }

        constexpr inline Color& operator*=(const sdl::Color& other)
        {
            *this = (*this * other);
            return *this;
        }

        constexpr inline Color& operator/=(const sdl::Color& other)
        {
            *this = (*this / other);
            return *this;
        }

    public:
        constexpr static inline Color lerp(const Color& s, const Color& e, u8 step)
        {
            return Color{ s + (e - s) * step };
        }

        void test_color_lerp()
        {
            sdl::Color start{ 255, 0, 0, 50 };
            sdl::Color end{ 0, 0, 255, 50 };

            u8 val = 0;
            while (++val < 255)
            {
                auto&& c = sdl::Color::lerp(start, end, val);
                fmt::print(c, "test\n");
                val += 1;
            }
        }

        // constexpr static inline color gradient(const std::vector<sdl::color>& colors, u8 step)
        // {
        //     const auto stop_len = 1 / (colors.size() - 1);
        //     const auto step_ratio = step / stop_len;
        //     const auto stop_idx = std::floor(step_ratio);
        //     if (stop_idx == (colors.size() - 1))
        //         return colors[colors.size() - 1];
        //     const auto end_step = step_ratio % 1;
        //     return lerp(colors[stop_idx], colors[stop_idx + 1], end_step);
        // }

        inline u32 rgb(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGB(format, this->r, this->g, this->b);
        }

        inline u32 rgba(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGBA(format, this->r, this->g, this->b, this->a);
        }

        constexpr inline bool operator==(const Color& other) const
        {
            return 0 == memory::static_memcmp<sizeof(*this)>(this, &other);
        }

        constexpr inline bool is_empty() const
        {
            return this->operator==({});
        }

        constexpr inline bool operator!=(const Color& other) const
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

        constexpr inline operator std::tuple<f32, f32, f32, f32>() const
        {
            return std::tuple{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        }

        constexpr inline operator std::array<f32, 4>() const
        {
            return std::array{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        }

        constexpr inline operator std::tuple<u8, u8, u8>() const
        {
            return { r, g, b };
        }

        constexpr static inline auto create(u8 r, u8 g, u8 b)
            -> std::remove_reference_t<decltype(std::declval<Color>())>
        {
            return std::tuple<u8, u8, u8>(r, g, b);
        }

    public:
        u8 r{ 0 };
        u8 g{ 0 };
        u8 b{ 0 };
        u8 a{ Preset::Opaque };
    };
}
