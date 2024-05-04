#pragma once

#include <algorithm>
#include <concepts>

#include <fmt/color.h>

#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"
#include "utils/random.hpp"
#include "utils/sdl_defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_pixels.h>
SDL_C_LIB_END

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
        requires rl::any_of<T, f32, u8>
    struct color
    {
    public:
        constexpr color(T red, T green, T blue, T alpha = 1.0f)
            : r{ red }
            , g{ green }
            , b{ blue }
            , a{ alpha }
        {
        }

        template <rl::integer I>
            requires std::same_as<T, f32>
        explicit constexpr color(I ri, I gi, I bi, I ai = 255)
            : r{ static_cast<T>(static_cast<u8>(ri) / 255.0f) }
            , g{ static_cast<T>(static_cast<u8>(gi) / 255.0f) }
            , b{ static_cast<T>(static_cast<u8>(bi) / 255.0f) }
            , a{ static_cast<T>(static_cast<u8>(ai) / 255.0f) }
        {
        }

        template <rl::floating_point F>
            requires std::same_as<T, u8>
        constexpr color(const F rf, const F gf, const F bf, const F af = 1.0f)
            : r{ static_cast<u8>(std::clamp(static_cast<f32>(rf) * 255.0f, 0.0f, 255.0f)) }
            , g{ static_cast<u8>(std::clamp(static_cast<f32>(gf) * 255.0f, 0.0f, 255.0f)) }
            , b{ static_cast<u8>(std::clamp(static_cast<f32>(bf) * 255.0f, 0.0f, 255.0f)) }
            , a{ static_cast<u8>(std::clamp(static_cast<f32>(af) * 255.0f, 0.0f, 255.0f)) }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr color(const u32 rgba)
            requires std::same_as<T, u8>
            : r{ static_cast<u8>(0xff & (rgba >> (8 * 3))) }
            , g{ static_cast<u8>(0xff & (rgba >> (8 * 2))) }
            , b{ static_cast<u8>(0xff & (rgba >> (8 * 1))) }
            , a{ static_cast<u8>(0xff & (rgba >> (8 * 0))) }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr color(const u32 rgba)
            requires std::same_as<T, f32>
            : r{ (static_cast<f32>(0xff & rgba >> 8 * 3) / 255.0f) }
            , g{ (static_cast<f32>(0xff & rgba >> 8 * 2) / 255.0f) }
            , b{ (static_cast<f32>(0xff & rgba >> 8 * 1) / 255.0f) }
            , a{ (static_cast<f32>(0xff & rgba >> 8 * 0) / 255.0f) }
        {
        }

        // 0x00RRGGBB
        explicit constexpr color(fmt::rgb rgb)
            requires std::same_as<T, u8>
            : r{ rgb.r }
            , g{ rgb.g }
            , b{ rgb.b }
            , a{ 255 }
        {
        }

        constexpr static color<u8> rand()
        {
            return color{
                static_cast<u8>(rl::random<0, 128>::value()),
                static_cast<u8>(rl::random<0, 128>::value()),
                static_cast<u8>(rl::random<0, 128>::value()),
                255,
            };
        }

    public:
        constexpr color<T> operator+(color<T> other) const
        {
            return color{
                this->r + other.r,
                this->g + other.g,
                this->b + other.b,
                this->a + other.a,
            };
        }

        constexpr color<T> operator-(color<T> other) const
        {
            return color{
                this->r - other.r,
                this->g - other.g,
                this->b - other.b,
                this->a - other.a,
            };
        }

        constexpr color<T> operator*(color<T> other) const
        {
            return color{
                this->r * other.r,
                this->g * other.g,
                this->b * other.b,
                this->a * other.a,
            };
        }

        constexpr color<T> operator/(color<T> other) const
        {
            return color{
                this->r / other.r,
                this->g / other.g,
                this->b / other.b,
                this->a / other.a,
            };
        }

        template <rl::numeric N>
        constexpr color<T> operator+(N val) const
        {
            return color{
                this->r + val,
                this->g + val,
                this->b + val,
                this->a + val,
            };
        }

        template <rl::numeric N>
        constexpr color<T> operator-(N val) const
        {
            return color{
                this->r - val,
                this->g - val,
                this->b - val,
                this->a - val,
            };
        }

        template <rl::numeric N>
        constexpr color<T> operator*(N val) const
        {
            return color{
                this->r * val,
                this->g * val,
                this->b * val,
                this->a * val,
            };
        }

        template <rl::numeric N>
        constexpr color<T> operator/(N val) const
        {
            return color{
                this->r / val,
                this->g / val,
                this->b / val,
                this->a / val,
            };
        }

        constexpr color<T>& operator+=(color<T> other)
        {
            this->r += other.r;
            this->g += other.g;
            this->b += other.b;
            this->a += other.a;
            return *this;
        }

        constexpr color<T>& operator-=(color<T> other)
        {
            this->r -= other.r;
            this->g -= other.g;
            this->b -= other.b;
            this->a -= other.a;
            return *this;
        }

        constexpr color<T>& operator*=(color<T> other)
        {
            this->r *= other.r;
            this->g *= other.g;
            this->b *= other.b;
            this->a *= other.a;
            return *this;
        }

        constexpr color<T>& operator/=(color<T> other)
        {
            this->r /= other.r;
            this->g /= other.g;
            this->b /= other.b;
            this->a /= other.a;
            return *this;
        }

    public:
        constexpr static color<T> lerp(color<T> s, color<T> e, T step)
        {
            return { s + (e - s) * step };
        }

        u32 rgb(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGB(format, this->r, this->g, this->b);
        }

        u32 rgba(const SDL3::SDL_PixelFormat* format) const
        {
            return SDL3::SDL_MapRGBA(format, this->r, this->g, this->b, this->a);
        }

        [[nodiscard]]
        constexpr bool is_empty() const
        {
            return *this == color<T>{};
        }

        [[nodiscard]]
        constexpr bool is_null() const
        {
            return math::equal(this->r, static_cast<T>(0)) &&
                   math::equal(this->g, static_cast<T>(0)) &&
                   math::equal(this->b, static_cast<T>(0)) &&
                   math::equal(this->a, static_cast<T>(0));
        }

    public:
        [[nodiscard]]
        constexpr color<f32> to_f32()
            requires std::same_as<T, u8>
        {
            return color<f32>{
                std::clamp(this->r / 255.0f, 0.0f, 1.0f),
                std::clamp(this->g / 255.0f, 0.0f, 1.0f),
                std::clamp(this->b / 255.0f, 0.0f, 1.0f),
                std::clamp(this->a / 255.0f, 0.0f, 1.0f),
            };
        }

        [[nodiscard]]
        constexpr color<u8> to_u8()
            requires std::same_as<T, f32>
        {
            return color<u8>{
                static_cast<u8>(std::clamp(static_cast<u8>(this->r) * 255.0f, 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<u8>(this->g) * 255.0f, 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<u8>(this->b) * 255.0f, 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<u8>(this->a) * 255.0f, 0.0f, 255.0f)),
            };
        }

        [[nodiscard]]
        constexpr operator ds::color<u8>()
            requires std::same_as<T, f32>
        {
            return color<u8>{
                static_cast<u8>(std::clamp(static_cast<f32>(this->r * 255.0f), 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<f32>(this->g * 255.0f), 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<f32>(this->b * 255.0f), 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<f32>(this->a * 255.0f), 0.0f, 255.0f)),
            };
        }

        [[nodiscard]]
        constexpr operator ds::color<u8>() const
            requires std::same_as<T, f32>
        {
            return color<u8>{
                static_cast<u8>(std::clamp(static_cast<f32>(this->r * 255.0f), 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<f32>(this->g * 255.0f), 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<f32>(this->b * 255.0f), 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(static_cast<f32>(this->a * 255.0f), 0.0f, 255.0f)),
            };
        }

        [[nodiscard]]
        constexpr operator ds::color<f32>()
            requires std::same_as<T, u8>
        {
            return ds::color<f32>{
                std::clamp(static_cast<u8>(this->r) / 255.0f, 0.0f, 1.0f),
                std::clamp(static_cast<u8>(this->g) / 255.0f, 0.0f, 1.0f),
                std::clamp(static_cast<u8>(this->b) / 255.0f, 0.0f, 1.0f),
                std::clamp(static_cast<u8>(this->a) / 255.0f, 0.0f, 1.0f),
            };
        }

        [[nodiscard]]
        constexpr operator ds::color<f32>() const
            requires std::same_as<T, u8>
        {
            return ds::color<f32>{
                std::clamp(static_cast<u8>(this->r) / 255.0f, 0.0f, 1.0f),
                std::clamp(static_cast<u8>(this->g) / 255.0f, 0.0f, 1.0f),
                std::clamp(static_cast<u8>(this->b) / 255.0f, 0.0f, 1.0f),
                std::clamp(static_cast<u8>(this->a) / 255.0f, 0.0f, 1.0f),
            };
        }

        [[nodiscard]]
        consteval operator fmt::rgb() const
            requires std::same_as<T, u8>
        {
            return fmt::rgb{ r, g, b };
        }

        [[nodiscard]]
        explicit constexpr operator fmt::text_style() const
        {
            return fmt::fg(static_cast<fmt::rgb>(*this));
        }

    public:
        T r{ 0 };
        T g{ 0 };
        T b{ 0 };
        T a{ rl::integer<T> ? 255 : 1.0f };
    };

#pragma pack()
}

namespace rl {
    struct Colors
    {
        constexpr static ds::color<f32> Transparent{ 0, 0, 0, 0 };
        constexpr static ds::color<f32> White{ 255, 255, 255, 255 };
        constexpr static ds::color<f32> LightGrey{ 192, 195, 201, 255 };
        constexpr static ds::color<f32> Grey{ 165, 165, 165, 255 };
        constexpr static ds::color<f32> DarkGrey{ 38, 43, 51, 255 };
        constexpr static ds::color<f32> DarkerGrey{ 51, 51, 51, 255 };
        constexpr static ds::color<f32> DarkererGrey{ 17, 17, 17, 255 };
        constexpr static ds::color<f32> Black{ 0, 0, 0, 255 };
        constexpr static ds::color<f32> Red{ 212, 164, 164, 255 };
        constexpr static ds::color<f32> Green{ 154, 175, 139, 255 };
        constexpr static ds::color<f32> Yellow{ 202, 183, 127, 255 };
        constexpr static ds::color<f32> Blue{ 119, 157, 201, 255 };
        constexpr static ds::color<f32> Purple{ 182, 173, 219, 255 };
        constexpr static ds::color<f32> Cyan{ 131, 178, 182, 255 };
        constexpr static ds::color<f32> Background{ 39, 43, 51, 255 };
    };
}

namespace rl::ds {
    template <typename T>
    constexpr auto format_as(const color<T>& c)
    {
        return fmt::format("(r={} g={} b={} a={})",
                           c.r, c.g, c.b, c.a);
    }
}
