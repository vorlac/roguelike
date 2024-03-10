#pragma once

#include "utils/numeric.hpp"

namespace rl {
    enum class Side : i8_fast {
        None = 0,

        Left = 1 << 0,
        Right = 1 << 1,
        Top = 1 << 2,
        Bottom = 1 << 3,

        TopLeft = Top | Left,
        TopRight = Top | Right,
        BottomLeft = Bottom | Left,
        BottomRight = Bottom | Right,
    };

    enum class Quad : i8_fast {
        TopLeft = Side::Top | Side::Left,
        BottomLeft = Side::Bottom | Side::Left,
        TopRight = Side::Top | Side::Right,
        BottomRight = Side::Bottom | Side::Right,
    };

    enum class Axis : i8_fast {
        Horizontal = 1,  // x axis
        Vertical = 2,    // y axis
    };

    enum class Direction : i8_fast {
        None = 0,

        North = 1 << 0,
        South = 1 << 1,
        East = 1 << 2,
        West = 1 << 3,

        NorthEast = North | East,
        SouthEast = South | East,
        NorthWest = North | West,
        SouthWest = South | West,
    };
}

namespace rl {
    constexpr auto format_as(const Side side)
    {
        switch (side)
        {
            case Side::None:
                return "None";
            case Side::Left:
                return "Left";
            case Side::Right:
                return "Right";
            case Side::Top:
                return "Top";
            case Side::Bottom:
                return "Bottom";
            case Side::TopLeft:
                return "TopLeft";
            case Side::TopRight:
                return "TopRight";
            case Side::BottomLeft:
                return "BottomLeft";
            case Side::BottomRight:
                return "BottomRight";
            default:
                return "Invalid";
        }
    }
}
