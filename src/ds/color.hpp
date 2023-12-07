#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/color.h>

#include "core/math.hpp"
#include "core/numeric.hpp"
#include "sdl/defs.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/memory.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_pixels.h>
SDL_C_LIB_END

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T = u8>
        requires rl::any_of<T, f32, u8>
    struct color
    {
    public:
        constexpr static inline T Transparent = T(0);
        constexpr static inline T Opaque = sizeof(T) == sizeof(u8) ? T(255) : T(1);

    public:
        constexpr inline color() = default;

        template <rl::integer I>
            requires(!std::same_as<T, I>)
        constexpr inline color(I cr, I cg, I cb, I ca = Opaque)
            : r{ cast::to<T>(cr) }
            , g{ cast::to<T>(cg) }
            , b{ cast::to<T>(cb) }
            , a{ cast::to<T>(ca) }
        {
        }

        template <rl::floating_point F>
        constexpr inline color(F cr, F cg, F cb, F ca = 1.0f)
            requires std::same_as<T, u8>
            : r{ static_cast<T>(std::clamp(cr * 255.0f, 0.0f, 255.0f)) }
            , g{ static_cast<T>(std::clamp(cg * 255.0f, 0.0f, 255.0f)) }
            , b{ static_cast<T>(std::clamp(cb * 255.0f, 0.0f, 255.0f)) }
            , a{ static_cast<T>(std::clamp(ca * 255.0f, 0.0f, 255.0f)) }
        {
        }

        constexpr inline color(std::tuple<T, T, T> tup)
            : r{ std::get<0>(tup) }
            , g{ std::get<1>(tup) }
            , b{ std::get<2>(tup) }
            , a{ Opaque }
        {
        }

        template <rl::integer I>
        constexpr inline color& operator=(std::tuple<I, I, I> tup)
        {
            r = cast::to<I>(std::get<0>(tup));
            g = cast::to<I>(std::get<1>(tup));
            b = cast::to<I>(std::get<2>(tup));
            return *this;
        }

        constexpr inline color(T cr, T cg, T cb, T ca = Opaque)
            : r{ cr }
            , g{ cg }
            , b{ cb }
            , a{ ca }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr inline color(u32 rgba)
            requires std::same_as<T, u8>
            : r{ cast::to<u8>(0xff & (rgba >> (8 * 3))) }
            , g{ cast::to<u8>(0xff & (rgba >> (8 * 2))) }
            , b{ cast::to<u8>(0xff & (rgba >> (8 * 1))) }
            , a{ cast::to<u8>(0xff & (rgba >> (8 * 0))) }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr inline color(u32 rgba)
            requires std::same_as<T, f32>
            : r{ cast::to<f32>((0xff & (rgba >> (8 * 3))) / 255.0f) }
            , g{ cast::to<f32>((0xff & (rgba >> (8 * 2))) / 255.0f) }
            , b{ cast::to<f32>((0xff & (rgba >> (8 * 1))) / 255.0f) }
            , a{ cast::to<f32>((0xff & (rgba >> (8 * 0))) / 255.0f) }
        {
        }

        // 0x00RRGGBB
        constexpr inline color(fmt::rgb rgb)
            requires std::same_as<T, u8>
            : r{ rgb.r }
            , g{ rgb.g }
            , b{ rgb.b }
            , a{ Opaque }
        {
        }

        constexpr inline color(const SDL3::SDL_Color& c)
            requires std::same_as<T, u8>
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr inline color(SDL3::SDL_Color&& c)
            requires std::same_as<T, u8>
            : r{ c.r }
            , g{ c.g }
            , b{ c.b }
            , a{ c.a }
        {
        }

        constexpr inline color<T> operator+(const color<T>& other) const
        {
            return {
                static_cast<T>(r + other.r),
                static_cast<T>(g + other.g),
                static_cast<T>(b + other.b),
                static_cast<T>(a + other.a),
            };
        }

        constexpr inline color<T> operator-(const color<T>& other) const
        {
            return {
                static_cast<T>(r - other.r),
                static_cast<T>(g - other.g),
                static_cast<T>(b - other.b),
                static_cast<T>(a - other.a),
            };
        }

        constexpr inline color<T> operator*(const color<T>& other) const
        {
            return {
                static_cast<T>(r * other.r),
                static_cast<T>(g * other.g),
                static_cast<T>(b * other.b),
                static_cast<T>(a * other.a),
            };
        }

        constexpr inline color<T> operator/(const color<T>& other) const
        {
            return {
                static_cast<T>(r / other.r),
                static_cast<T>(g / other.g),
                static_cast<T>(b / other.b),
                static_cast<T>(a / other.a),
            };
        }

        template <rl::numeric N>
        constexpr inline color<T> operator+(const N val) const
        {
            return {
                static_cast<T>(r + val),
                static_cast<T>(g + val),
                static_cast<T>(b + val),
                static_cast<T>(a + val),
            };
        }

        template <rl::numeric N>
        constexpr inline color<T> operator-(const N val) const
        {
            return {
                static_cast<T>(r - val),
                static_cast<T>(g - val),
                static_cast<T>(b - val),
                static_cast<T>(a - val),
            };
        }

        template <rl::numeric N>
        constexpr inline color<T> operator*(const N val) const
        {
            return {
                static_cast<T>(r * val),
                static_cast<T>(g * val),
                static_cast<T>(b * val),
                static_cast<T>(a * val),
            };
        }

        template <rl::numeric N>
        constexpr inline color<T> operator/(const N val) const
        {
            return {
                static_cast<T>(r / val),
                static_cast<T>(g / val),
                static_cast<T>(b / val),
                static_cast<T>(a / val),
            };
        }

        constexpr inline color<T>& operator+=(const color<T>& other)
        {
            *this = (*this + other);
            return *this;
        }

        constexpr inline color<T>& operator-=(const color<T>& other)
        {
            *this = (*this - other);
            return *this;
        }

        constexpr inline color<T>& operator*=(const color<T>& other)
        {
            *this = (*this * other);
            return *this;
        }

        constexpr inline color<T>& operator/=(const color<T>& other)
        {
            *this = (*this / other);
            return *this;
        }

    public:
        constexpr static inline color<T> lerp(const color<T>& s, const color<T>& e, T step)
        {
            return { s + (e - s) * step };
        }

        // static void test_color_lerp()
        // {
        //     color<u8>start{ 255, 0, 0, 50 };
        //     color<u8>end{ 0, 0, 255, 50 };

        //     uint8_t val{ 0 };
        //     while (++val < 255)
        //     {
        //         auto&& c = color::lerp(start, end, val);
        //         fmt::print(c, "||");
        //     }
        //     fmt::print("\n");
        // }

        // constexpr static inline color gradient(const std::vector<color>& colors, T step)
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

        constexpr inline operator color<f32>() const
            requires rl::integer<T>
        {
            return std::array<T, 4>{ r, g, b, a };
        }

        constexpr inline operator std::array<T, 4>() const
        {
            return std::array<T, 4>{ r, g, b, a };
        }

        constexpr inline operator std::tuple<T, T, T, T>() const
        {
            return { r, g, b, a };
        }

        constexpr inline operator std::tuple<f32, f32, f32, f32>() const
            requires std::same_as<T, u8>
        {
            return std::tuple{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        }

        constexpr inline operator std::array<f32, 4>() const
            requires std::same_as<T, u8>
        {
            return std::array{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        }

        constexpr inline operator std::tuple<T, T, T>() const
        {
            return { r, g, b };
        }

        constexpr static inline auto create(T r, T g, T b)
            -> std::remove_reference_t<decltype(std::declval<color>())>
        {
            return std::tuple<T, T, T>(r, g, b);
        }

    public:
        T r{ 0 };
        T g{ 0 };
        T b{ 0 };
        T a{ Opaque };
    };

#pragma pack()
}

namespace rl {
    struct Colors
    {
        constexpr static inline ds::color<f32> White{ 0xFFFFFFFF };
        constexpr static inline ds::color<f32> LightGrey{ 0xC1C4CAFF };
        constexpr static inline ds::color<f32> Grey{ 0xA6A6A6FF };
        constexpr static inline ds::color<f32> DarkGrey{ 0x262B33FF };
        constexpr static inline ds::color<f32> DarkerGrey{ 0x333333FF };
        constexpr static inline ds::color<f32> Black{ 0x000000FF };

        constexpr static inline ds::color<f32> Red{ 0xD4A4A4FF };
        constexpr static inline ds::color<f32> Green{ 0x9AAF8BFF };
        constexpr static inline ds::color<f32> Yellow{ 0xCAB880FF };
        constexpr static inline ds::color<f32> Blue{ 0x779DC9FF };
        constexpr static inline ds::color<f32> Purple{ 0xB6ADDBFF };
        constexpr static inline ds::color<f32> Cyan{ 0x83B2B6FF };
    };
}
