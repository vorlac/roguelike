#pragma once

#include "utils/numeric.hpp"

// clang-format off

namespace rl::ui {
    enum class Interaction : i16_fast {
        None      = 0x00,  // constant positionioning

        Propagate = 0x01,  // pass unhandled events to children
        Move      = 0x02,  // being moved or can be moved
        Drag      = 0x04,  // grbbed & dragging a widget
        Resize    = 0x08,  // being resized or can be resized
        Dock      = 0x10,  // dock to a side of the screen
        Merge     = 0x20,  // merge dialog into another as tabs
        Modal     = 0x40,  // blocks all events outside of scope

        All       = 0xFF,  // all ui interactions
    };

    enum class Component : i16_fast {
        None,
        Header,
        Body,
        Scrollbar,
        Edge,
    };

}

namespace rl {
    enum class Side : i16_fast {
        None        = 0,
        Left        = 1 << 0,
        Right       = 1 << 1,
        Top         = 1 << 2,
        Bottom      = 1 << 3,
        TopLeft     = Top    | Left,
        TopRight    = Top    | Right,
        BottomLeft  = Bottom | Left,
        BottomRight = Bottom | Right,
    };

    enum class Quad : i16_fast {
        TopLeft     = Side::Top    | Side::Left,
        TopRight    = Side::Top    | Side::Right,
        BottomLeft  = Side::Bottom | Side::Left,
        BottomRight = Side::Bottom | Side::Right,
    };

    enum class Axis : i16_fast {
        Horizontal = 1 << 0,  // x axis
        Vertical   = 1 << 1,  // y axis
    };

    enum class Direction : i16_fast {
        None      = 0,
        North     = 1 << 0,
        South     = 1 << 1,
        East      = 1 << 2,
        West      = 1 << 3,
        NorthEast = North | East,
        SouthEast = South | East,
        NorthWest = North | West,
        SouthWest = South | West,
    };
}

// clang-format on

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
