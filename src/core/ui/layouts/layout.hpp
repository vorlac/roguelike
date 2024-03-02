#pragma once

#include <string>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    class Widget;

    enum class Alignment {
        Unknown = -1,  // Invalid / uninitialized alignment
        Minimum = 0,   // Take only as much space as is required.
        Center,        // Center align.
        Maximum,       // Take as much space as is allowed.
        Fill           // Fill according to preferred sizes.
    };

    enum class Orientation {
        Unknown = -1,    // Invalid / uninitialized orientation
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    enum Axis {
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    struct Anchor
    {
    public:
        Anchor() = default;

        Anchor(const u32 x, const u32 y, const Alignment horiz = Alignment::Fill,
               const Alignment vert = Alignment::Fill)
            : grid_pos{ static_cast<u8>(x), static_cast<u8>(y) }
            , cell_size{ 1, 1 }
            , align{ horiz, vert }
        {
        }

        Anchor(const u32 grid_x, const u32 grid_y, const u32 cell_span_width,
               const u32 cell_span_height, const Alignment horizontal_alignment = Alignment::Fill,
               const Alignment vertical_alignment = Alignment::Fill)
            : grid_pos{ grid_x, grid_y }
            , cell_size{ cell_span_width, cell_span_height }
            , align{ horizontal_alignment, vertical_alignment }
        {
        }

    public:
        ds::point<u32> grid_pos{ 0, 0 };
        ds::dims<u32> cell_size{ 0, 0 };
        std::array<Alignment, 2> align{};
    };

    class Layout : public ds::refcounted
    {
    public:
        template <typename T>
        struct Spacing
        {
            T horizontal{};
            T vertical{};
        };

    public:
        // Performs applies all Layout computations for the given widget.
        virtual void perform_layout(nvg::Context* nvc, Widget* w) const = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> preferred_size(nvg::Context* nvc, const Widget* w) const = 0;

    protected:
        std::string name() const
        {
            return typeid(*this).name();
        }
    };
}

namespace rl::ui {
    constexpr static auto format_as(const Alignment alignment)
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

    constexpr static auto format_as(const Orientation orientation)
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

    constexpr static auto format_as(const Axis axis)
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

    static auto format_as(const Anchor& anchor)
    {
        return fmt::format("Format[pos=({}), size=({}), align=(h:{}, v:{})]", anchor.grid_pos,
                           anchor.cell_size, anchor.align[Axis::Horizontal],
                           anchor.align[Axis::Vertical]);
    }
}
