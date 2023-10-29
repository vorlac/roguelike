#pragma once

// #include <string>

#include "core/ds/color.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numerics.hpp"

namespace rl::ui
{
    enum justification : u16_fast {
        Top         = 1 << 0,
        Bottom      = 1 << 1,
        Left        = 1 << 2,
        Right       = 1 << 3,
        Centered    = 1 << 4,
        TopLeft     = Top | Left,
        TopRight    = Top | Right,
        BottomLeft  = Bottom | Left,
        BottomRight = Bottom | Right,
    };

    struct style
    {
        rl::color bg_color{};
        rl::color fg_color{};
        rl::color fill_color{};
        rl::color border_color{};
        rl::color border_style{};
        rl::color border_thickness{};
        rl::color text_fg_color{};
        rl::color text_bg_color{};
        // raylib::FontType font_type{};
        // raylib::Font font{};
    };

    struct layout_info
    {
        enum class orientation : u16_fast {
            None       = 0,
            Horizontal = 1,
            Vertical   = 2,
            Grid       = 3,
        };

        struct matrix
        {
            static constexpr inline u16_fast DynamicScaling{ 0 };

            struct constraints
            {
                u16_fast cols = matrix::DynamicScaling;
                u16_fast rows = matrix::DynamicScaling;
            };

            // defines the minimum number of rows and columns
            // that can/will be created within a layout
            constraints max = {
                .cols = matrix::DynamicScaling,
                .rows = matrix::DynamicScaling,
            };
            // defines the maximum number of rows and columns
            // that can/will be created within a layout
            constraints min = {
                .cols = matrix::DynamicScaling,
                .rows = matrix::DynamicScaling,
            };
        };

        orientation orientation{ orientation::None };
        matrix::constraints max = {
            .cols = matrix::DynamicScaling,
            .rows = matrix::DynamicScaling,
        };
        matrix::constraints min = {
            .cols = matrix::DynamicScaling,
            .rows = matrix::DynamicScaling,
        };
    };

    struct margins
    {
        u16_fast top    = 0;
        u16_fast bottom = 0;
        u16_fast left   = 0;
        u16_fast right  = 0;
    };

    struct properties
    {
        std::string text{};
        ds::dimensions<int32_t> size{ 0, 0 };
        ds::point<int32_t> position{ 0, 0 };
        ui::justification justification{ justification::Centered };
        ui::margins inner_margin{};
        ui::margins outer_margin{};
        ui::style style{};
    };
}
