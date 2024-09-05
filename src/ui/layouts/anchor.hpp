#pragma once

#include <array>

#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "utils/properties.hpp"

namespace rl::ui {

    struct Anchor {
    public:
        Anchor() = default;

        Anchor(const u32 x, const u32 y,
               const Placement_OldAlignment horiz = Placement_OldAlignment::Fill,
               const Placement_OldAlignment vert = Placement_OldAlignment::Fill)
            : grid_pos{ static_cast<u8>(x), static_cast<u8>(y) }
            , cell_size{ 1, 1 }
            , align{ horiz, vert } {
        }

        Anchor(const u32 grid_x, const u32 grid_y, const u32 cell_span_width,
               const u32 cell_span_height,
               const Placement_OldAlignment horizontal_alignment = Placement_OldAlignment::Fill,
               const Placement_OldAlignment vertical_alignment = Placement_OldAlignment::Fill)
            : grid_pos{ grid_x, grid_y }
            , cell_size{ cell_span_width, cell_span_height }
            , align{ horizontal_alignment, vertical_alignment } {
        }

    public:
        ds::point<u32> grid_pos{ 0, 0 };
        ds::dims<u32> cell_size{ 0, 0 };
        std::array<Placement_OldAlignment, 2> align{};
    };
}
