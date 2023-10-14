#pragma once
#include <array>
#include <raylib.h>

namespace rl
{
    struct color
    {
        static constexpr inline auto lightgray{ LIGHTGRAY };
        static constexpr inline auto gray{ GRAY };
        static constexpr inline auto darkgray{ DARKGRAY };
        static constexpr inline auto yellow{ YELLOW };
        static constexpr inline auto gold{ GOLD };
        static constexpr inline auto orange{ ORANGE };
        static constexpr inline auto pink{ PINK };
        static constexpr inline auto red{ RED };
        static constexpr inline auto maroon{ MAROON };
        static constexpr inline auto green{ GREEN };
        static constexpr inline auto lime{ LIME };
        static constexpr inline auto darkgreen{ DARKGREEN };
        static constexpr inline auto skyblue{ SKYBLUE };
        static constexpr inline auto blue{ BLUE };
        static constexpr inline auto darkblue{ DARKBLUE };
        static constexpr inline auto purple{ PURPLE };
        static constexpr inline auto violet{ VIOLET };
        static constexpr inline auto darkpurple{ DARKPURPLE };
        static constexpr inline auto beige{ BEIGE };
        static constexpr inline auto brown{ BROWN };
        static constexpr inline auto darkbrown{ DARKBROWN };
        static constexpr inline auto white{ WHITE };
        static constexpr inline auto black{ BLACK };
        static constexpr inline auto blank{ BLANK };
        static constexpr inline auto magenta{ MAGENTA };
        static constexpr inline auto raywhite{ RAYWHITE };
    };

    constexpr Color rand_color(int32_t val)
    {
        constexpr std::array color_list = {
            color::lightgray, color::gray,      color::darkgray,   color::yellow, color::gold,
            color::orange,    color::pink,      color::red,        color::maroon, color::green,
            color::lime,      color::darkgreen, color::skyblue,    color::blue,   color::darkblue,
            color::purple,    color::violet,    color::darkpurple, color::beige,  color::brown,
            color::darkbrown, color::white,     color::black,      color::blank,  color::magenta,
            color::raywhite,
        };

        return color_list.at(val % color_list.size());
    }
}
