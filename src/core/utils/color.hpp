#pragma once

#include <array>
#include <utility>

#include "thirdparty/raylib.hpp"

namespace rl
{
    struct color : public raylib::Color
    {
        constexpr operator raylib::Color() const
        {
            return static_cast<raylib::Color>(*this);
        }

        static constexpr inline auto lightgray{ raylib::LIGHTGRAY };
        static constexpr inline auto gray{ raylib::GRAY };
        static constexpr inline auto darkgray{ raylib::DARKGRAY };
        static constexpr inline auto yellow{ raylib::YELLOW };
        static constexpr inline auto gold{ raylib::GOLD };
        static constexpr inline auto orange{ raylib::ORANGE };
        static constexpr inline auto pink{ raylib::PINK };
        static constexpr inline auto red{ raylib::RED };
        static constexpr inline auto maroon{ raylib::MAROON };
        static constexpr inline auto green{ raylib::GREEN };
        static constexpr inline auto lime{ raylib::LIME };
        static constexpr inline auto darkgreen{ raylib::DARKGREEN };
        static constexpr inline auto skyblue{ raylib::SKYBLUE };
        static constexpr inline auto blue{ raylib::BLUE };
        static constexpr inline auto darkblue{ raylib::DARKBLUE };
        static constexpr inline auto purple{ raylib::PURPLE };
        static constexpr inline auto violet{ raylib::VIOLET };
        static constexpr inline auto darkpurple{ raylib::DARKPURPLE };
        static constexpr inline auto beige{ raylib::BEIGE };
        static constexpr inline auto brown{ raylib::BROWN };
        static constexpr inline auto darkbrown{ raylib::DARKBROWN };
        static constexpr inline auto white{ raylib::WHITE };
        static constexpr inline auto black{ raylib::BLACK };
        static constexpr inline auto blank{ raylib::BLANK };
        static constexpr inline auto magenta{ raylib::MAGENTA };
        static constexpr inline auto raywhite{ raylib::RAYWHITE };
    };

    constexpr rl::color rand_color(int32_t val)
    {
        constexpr auto color_list = std::to_array<const raylib::Color>({
            color::lightgray, color::gray,      color::darkgray,   color::yellow, color::gold,
            color::orange,    color::pink,      color::red,        color::maroon, color::green,
            color::lime,      color::darkgreen, color::skyblue,    color::blue,   color::darkblue,
            color::purple,    color::violet,    color::darkpurple, color::beige,  color::brown,
            color::darkbrown, color::white,     color::black,      color::blank,  color::magenta,
            color::raywhite,
        });

        return static_cast<rl::color>(color_list.at(val % color_list.size()));
    }
}