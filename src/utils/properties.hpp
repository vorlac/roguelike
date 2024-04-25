#pragma once

#include "utils/conversions.hpp"
#include "utils/numeric.hpp"

// clang-format off

namespace rl {
    // Defines the different types of mouse
    // interaction states for the main Canvas
    enum class Interaction : u8_fast {
        None      = 0x0000,  // constant positionioning
        Propagate = 1 << 0,  // pass unhandled events to children
        Move      = 1 << 1,  // being moved or can be moved
        Drag      = 1 << 2,  // grbbed & dragging a widget
        Resize    = 1 << 3,  // being resized or can be resized
        Dock      = 1 << 4,  // dock to a side of the screen
        Merge     = 1 << 5,  // merge dialog into another as tabs
        Modal     = 1 << 6,  // blocks all events outside of scope
        All       = 0x00FF,  // All Interactions Modes
    };

    // Defines each potential UI widget component
    // Typically used for event/mouse handling
    enum class Component : u8_fast {
        None      = 0x0000,
        Header    = 1 << 0,
        Body      = 1 << 1,
        Scrollbar = 1 << 2,
        Edge      = 1 << 3,
    };

    enum class SizePolicy : u8_fast {
        Inherit    = 0x0000,
        FixedSize  = 1 << 0,
        Minimum    = 1 << 1,
        Maximum    = 1 << 2,
        Prefered   = 1 << 3,
    };
    // TODO: remove
    enum class Placement_OldAlignment : u8_fast {
        None       = 0x0000,  // Invalid / uninitialized alignment
        Minimum    = 1 << 0,  // Take only as much space as is required.
        Center     = 1 << 1,  // Center align.
        Maximum    = 1 << 2,  // Take as much space as is allowed.
        Fill       = 1 << 3,  // Fill according to preferred sizes.
    };

    enum class Alignment : u8_fast {
        None       = 0x0000,  // Invalid / uninitialized orientation
        Horizontal = 1 << 0,  // Layout expands on horizontal axis.
        Vertical   = 1 << 1,  // Layout expands on vertical axis.
    };

    enum class Align : u8_fast {
        None       = 0x0000,
        HLeft      = 1 << 0,  // Default, align text horizontally to left.
        HCenter    = 1 << 1,  // Align text horizontally to center.
        HRight     = 1 << 2,  // Align text horizontally to right.
        VTop       = 1 << 3,  // Align text vertically to top.
        VMiddle    = 1 << 4,  // Align text vertically to middle.
        VBottom    = 1 << 5,  // Align text vertically to bottom.
        VBaseline  = 1 << 6,  // Default, align text vertically to baseline.
    };

    enum class Axis : u8_fast {
        Horizontal = 1 << 0,  // x axis
        Vertical   = 1 << 1,  // y axis
    };

    enum class Outline : u8_fast {
        Inner  = 1 << 0,
        Outer  = 1 << 1,
    };

    enum class Side : u8_fast {
        None        = 0x0000,
        Left        = 1 << 0,
        Right       = 1 << 1,
        Top         = 1 << 2,
        Bottom      = 1 << 3,
        TopLeft     = Top    | Left,
        TopRight    = Top    | Right,
        BottomLeft  = Bottom | Left,
        BottomRight = Bottom | Right,
    };

    enum class Direction : u8_fast {
        None  = 0x0000,
        Up    = 1 << 0,
        Down  = 1 << 1,
        Left  = 1 << 2,
        Right = 1 << 3,
    };

    enum class Quad : u8_fast {
        TopLeft     = std::to_underlying(Side::Top    | Side::Left),
        TopRight    = std::to_underlying(Side::Top    | Side::Right),
        BottomLeft  = std::to_underlying(Side::Bottom | Side::Left),
        BottomRight = std::to_underlying(Side::Bottom | Side::Right),
    };

    enum class CompassDirection : u8_fast {
        None      = 0x0000,
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
        switch (side) {
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

    constexpr auto format_as(const Placement_OldAlignment alignment)
    {
        switch (alignment) {
            case Placement_OldAlignment::None:
                return "None";
            case Placement_OldAlignment::Minimum:
                return "Minimum";
            case Placement_OldAlignment::Center:
                return "Center";
            case Placement_OldAlignment::Maximum:
                return "Maximum";
            case Placement_OldAlignment::Fill:
                return "Fill";
        }

        return "Unknown";
    }

    constexpr auto format_as(const Alignment orientation)
    {
        switch (orientation) {
            case Alignment::None:
                return "None";
            case Alignment::Horizontal:
                return "Horizontal";
            case Alignment::Vertical:
                return "Vertical";
        }

        return "Unknown";
    }

    constexpr auto format_as(const Axis axis)
    {
        switch (axis) {
            case Axis::Horizontal:
                return "Horizontal";
            case Axis::Vertical:
                return "Vertical";
        }

        return "Unknown";
    }
}
