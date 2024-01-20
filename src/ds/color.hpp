#pragma once
#include <algorithm>
#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/color.h>

#include "graphics/vg/nanovg.hpp"
#include "sdl/defs.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/math.hpp"
#include "utils/memory.hpp"
#include "utils/numeric.hpp"

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
        constexpr static inline T Transparent = T(0);
        constexpr static inline T Opaque = sizeof(T) == sizeof(u8) ? T(255) : T(1);

    public:
        constexpr inline color() = default;

        constexpr inline color(T cr, T cg, T cb, T ca = Opaque)
            : r{ cr }
            , g{ cg }
            , b{ cb }
            , a{ ca }
        {
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr inline color(I ri, I gi, I bi, I ai = Opaque)
            : r{ static_cast<T>(ri / 255.0f) }
            , g{ static_cast<T>(gi / 255.0f) }
            , b{ static_cast<T>(bi / 255.0f) }
            , a{ static_cast<T>(ai / 255.0f) }
        {
        }

        template <rl::floating_point F>
            requires rl::integer<T>
        explicit constexpr inline color(F rf, F gf, F bf, F af = 1.0f)
            : r{ static_cast<T>(std::clamp(rf * 255.0f, 0.0f, 255.0f)) }
            , g{ static_cast<T>(std::clamp(gf * 255.0f, 0.0f, 255.0f)) }
            , b{ static_cast<T>(std::clamp(bf * 255.0f, 0.0f, 255.0f)) }
            , a{ static_cast<T>(std::clamp(af * 255.0f, 0.0f, 255.0f)) }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr inline color(u32 rgba)
            requires std::same_as<T, u8>
            : r{ static_cast<u8>(0xff & (rgba >> (8 * 3))) }
            , g{ static_cast<u8>(0xff & (rgba >> (8 * 2))) }
            , b{ static_cast<u8>(0xff & (rgba >> (8 * 1))) }
            , a{ static_cast<u8>(0xff & (rgba >> (8 * 0))) }
        {
        }

        // 0xRRGGBBAA
        explicit constexpr inline color(u32 rgba)
            requires std::same_as<T, f32>
            : r{ static_cast<f32>((0xff & (rgba >> (8 * 3))) / 255.0f) }
            , g{ static_cast<f32>((0xff & (rgba >> (8 * 2))) / 255.0f) }
            , b{ static_cast<f32>((0xff & (rgba >> (8 * 1))) / 255.0f) }
            , a{ static_cast<f32>((0xff & (rgba >> (8 * 0))) / 255.0f) }
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

    public:
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

    public:
        constexpr inline color<f32> to_f32() const
            requires std::same_as<T, u8>
        {
            return color<f32>{
                static_cast<f32>(std::clamp(this->r / 255.0f, 0.0f, 1.0f)),
                static_cast<f32>(std::clamp(this->g / 255.0f, 0.0f, 1.0f)),
                static_cast<f32>(std::clamp(this->b / 255.0f, 0.0f, 1.0f)),
                static_cast<f32>(std::clamp(this->a / 255.0f, 0.0f, 1.0f)),
            };
        }

        constexpr inline color<u8> to_u8() const
            requires std::same_as<T, f32>
        {
            return color<u8>{
                static_cast<u8>(std::clamp(this->r * 255.0f, 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(this->g * 255.0f, 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(this->b * 255.0f, 0.0f, 255.0f)),
                static_cast<u8>(std::clamp(this->a * 255.0f, 0.0f, 255.0f)),
            };
        }

        constexpr inline operator const nvg::NVGcolor() const
            requires std::same_as<T, f32>
        {
            return {
                this->r,
                this->g,
                this->b,
                this->a,
            };
        }

        constexpr inline operator nvg::NVGcolor() const
            requires std::same_as<T, u8>
        {
            return {
                static_cast<f32>(std::clamp(this->r / 255.0f, 0.0f, 1.0f)),
                static_cast<f32>(std::clamp(this->g / 255.0f, 0.0f, 1.0f)),
                static_cast<f32>(std::clamp(this->b / 255.0f, 0.0f, 1.0f)),
                static_cast<f32>(std::clamp(this->a / 255.0f, 0.0f, 1.0f)),
            };
        }

        constexpr inline operator fmt::rgb() const
            requires rl::integer<T>
        {
            return fmt::rgb{ r, g, b };
        }

        constexpr inline operator fmt::text_style() const
        {
            return fmt::fg(static_cast<fmt::rgb>(*this));
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
        constexpr static inline ds::color<f32> White{ 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr static inline ds::color<f32> LightGrey{ 0.756862f, 0.768627f, 0.792156f, 1.0f };
        constexpr static inline ds::color<f32> Grey{ 0.65098f, 0.65098f, 0.65098f, 1.0f };
        constexpr static inline ds::color<f32> DarkGrey{ 0.14901961f, 0.16862746f, 0.2f, 1.0f };
        constexpr static inline ds::color<f32> DarkerGrey{ 0.2f, 0.2f, 0.2f, 1.0f };
        constexpr static inline ds::color<f32> DarkererGrey{ 0.066667f, 0.066667f, 0.066667f, 1.0f };
        constexpr static inline ds::color<f32> Black{ 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr static inline ds::color<f32> Red{ 0.83137256f, 0.6431373f, 0.6431373f, 1.0f };
        constexpr static inline ds::color<f32> Green{ 0.6039216f, 0.6862745f, 0.54509807f, 1.0f };
        constexpr static inline ds::color<f32> Yellow{ 0.7921569f, 0.7215686f, 0.501960f, 1.0f };
        constexpr static inline ds::color<f32> Blue{ 0.46666667f, 0.6156863f, 0.7882353f, 1.0f };
        constexpr static inline ds::color<f32> Purple{ 0.7137255f, 0.6784314f, 0.85882354f, 1.0f };
        constexpr static inline ds::color<f32> Cyan{ 0.5137255f, 0.69803923f, 0.7137255f, 1.0f };
        constexpr static inline ds::color<f32> Background{ 0.156862f, 0.172549f, 0.203921f, 1.0f };
    };
}
