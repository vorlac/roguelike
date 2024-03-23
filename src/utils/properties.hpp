#pragma once

#include "utils/numeric.hpp"

// clang-format off

namespace rl {
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

    enum class Alignment : i16_fast{
        Unknown = -1,  // Invalid / uninitialized alignment
        Minimum = 0,   // Take only as much space as is required.
        Center,        // Center align.
        Maximum,       // Take as much space as is allowed.
        Fill           // Fill according to preferred sizes.
    };

    enum class Orientation : i16_fast{
        Unknown = -1,    // Invalid / uninitialized orientation
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    enum class Axis : i16_fast {
        Horizontal = 1 << 0,  // x axis
        Vertical   = 1 << 1,  // y axis
    };

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

    constexpr auto format_as(Side side)
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

    constexpr auto format_as(Alignment alignment)
    {
        switch (alignment)
        {
            case Alignment::Unknown:
                return "Unknown";
            case Alignment::Minimum:
                return "Minimum";
            case Alignment::Center:
                return "Center";
            case Alignment::Maximum:
                return "Maximum";
            case Alignment::Fill:
                return "Fill";
        }

        return "Unknown";
    }

    constexpr auto format_as(Orientation orientation)
    {
        switch (orientation)
        {
            case Orientation::Unknown:
                return "Unknown";
            case Orientation::Horizontal:
                return "Horizontal";
            case Orientation::Vertical:
                return "Vertical";
        }

        return "Unknown";
    }

    constexpr auto format_as(Axis axis)
    {
        switch (axis)
        {
            case Axis::Horizontal:
                return "Horizontal";
            case Axis::Vertical:
                return "Vertical";
        }

        return "Unknown";
    }
}
