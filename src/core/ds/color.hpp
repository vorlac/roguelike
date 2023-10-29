#pragma once

#include <tuple>
#include <type_traits>

#include "core/numerics.hpp"

#include "thirdparty/raygui.hpp"

namespace rl
{
    struct color
    {
        u8 r{ 0 };  // Color red value
        u8 g{ 0 };  // Color green value
        u8 b{ 0 };  // Color blue value
        u8 a{ 0 };  // Color alpha value

        color() = default;

        constexpr color(u8 _r, u8 _g, u8 _b, u8 _a)
            : r{ _r }
            , g{ _g }
            , b{ _b }
            , a{ _a }
        {
        }

        operator decltype(auto)()
        {
            return std::forward_as_tuple(r, g, b, a);
        }
    };

    namespace colors
    {
        static constexpr inline rl::color lightgray = rl::color{ 200, 200, 200, 255 };  // Light Gray
        static constexpr inline rl::color gray      = rl::color{ 130, 130, 130, 255 };  // Gray
        static constexpr inline rl::color darkgray  = rl::color{ 80, 80, 80, 255 };     // Dark Gray
        static constexpr inline rl::color yellow    = rl::color{ 253, 249, 0, 255 };    // Yellow
        static constexpr inline rl::color gold      = rl::color{ 255, 203, 0, 255 };    // Gold
        static constexpr inline rl::color orange    = rl::color{ 255, 161, 0, 255 };    // Orange
        static constexpr inline rl::color pink      = rl::color{ 255, 109, 194, 255 };  // Pink
        static constexpr inline rl::color red       = rl::color{ 230, 41, 55, 255 };    // Red
        static constexpr inline rl::color maroon    = rl::color{ 190, 33, 55, 255 };    // Maroon
        static constexpr inline rl::color green     = rl::color{ 0, 228, 48, 255 };     // Green
        static constexpr inline rl::color lime      = rl::color{ 0, 158, 47, 255 };     // Lime
        static constexpr inline rl::color darkgreen = rl::color{ 0, 117, 44, 255 };  // Dark Green
        static constexpr inline rl::color skyblue   = rl::color{ 102, 191, 255, 255 };  // Sky Blue
        static constexpr inline rl::color blue      = rl::color{ 0, 121, 241, 255 };    // Blue
        static constexpr inline rl::color darkblue  = rl::color{ 0, 82, 172, 255 };     // Dark Blue
        static constexpr inline rl::color purple    = rl::color{ 200, 122, 255, 255 };  // Purple
        static constexpr inline rl::color violet    = rl::color{ 135, 60, 190, 255 };   // Violet
        static constexpr inline rl::color darkpurple = rl::color{ 112, 31, 126, 255 };  // Dark Purple
        static constexpr inline rl::color beige     = rl::color{ 211, 176, 131, 255 };  // Beige
        static constexpr inline rl::color brown     = rl::color{ 127, 106, 79, 255 };   // Brown
        static constexpr inline rl::color darkbrown = rl::color{ 76, 63, 47, 255 };  // Dark Brown
        static constexpr inline rl::color white     = rl::color{ 255, 255, 255, 255 };  // White
        static constexpr inline rl::color black     = rl::color{ 0, 0, 0, 255 };        // Black
        static constexpr inline rl::color blank   = rl::color{ 0, 0, 0, 0 };  // Blank (Transparent)
        static constexpr inline rl::color magenta = rl::color{ 255, 0, 255, 255 };     // Magenta
        static constexpr inline rl::color raywhite = rl::color{ 245, 245, 245, 255 };  // My own White
    }

    constexpr color rand_color(i32 val)
    {
        constexpr std::array color_list{
            colors::lightgray, colors::gray,       colors::darkgray, colors::yellow,
            colors::gold,      colors::orange,     colors::pink,     colors::red,
            colors::maroon,    colors::green,      colors::lime,     colors::darkgreen,
            colors::skyblue,   colors::blue,       colors::darkblue, colors::purple,
            colors::violet,    colors::darkpurple, colors::beige,    colors::brown,
            colors::darkbrown, colors::white,      colors::black,    colors::blank,
            colors::magenta,   colors::raywhite,
        };

        return color_list.at(static_cast<u64>(val) % color_list.size());
    }
}
