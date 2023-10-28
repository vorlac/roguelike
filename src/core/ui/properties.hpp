#pragma once

#include <cstdint>
#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"

namespace rl::ui
{
    enum Justification : uint_fast16_t {
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

    struct Layout
    {
        enum Orientation : uint_fast16_t {
            None       = 0,
            Horizontal = 1,
            Vertical   = 2,
            Grid       = 3,
        };

        struct Matrix
        {
            static constexpr inline uint_fast16_t DynamicScaling{ 0 };

            struct Constraints
            {
                uint_fast16_t cols = Matrix::DynamicScaling;
                uint_fast16_t rows = Matrix::DynamicScaling;
            };

            // defines the minimum number of rows and columns
            // that can/will be created within a layout
            Constraints max = {
                .cols = Matrix::DynamicScaling,
                .rows = Matrix::DynamicScaling,
            };
            // defines the maximum number of rows and columns
            // that can/will be created within a layout
            Constraints min = {
                .cols = Matrix::DynamicScaling,
                .rows = Matrix::DynamicScaling,
            };
        };

        Orientation orientation{ Orientation::None };
        Matrix::Constraints max = {
            .cols = Matrix::DynamicScaling,
            .rows = Matrix::DynamicScaling,
        };
        Matrix::Constraints min = {
            .cols = Matrix::DynamicScaling,
            .rows = Matrix::DynamicScaling,
        };
    };

    struct Margins
    {
        uint_fast16_t top    = 0;
        uint_fast16_t bottom = 0;
        uint_fast16_t left   = 0;
        uint_fast16_t right  = 0;
    };

    struct Properties
    {
        std::string text{};
        ds::dimensions<int32_t> size{ 0, 0 };
        ds::point<int32_t> position{ 0, 0 };
        // identifies how the control ui element should align in it's parent
        Justification justification = Justification::Centered;
        // defines how the control should organize any children as they're added
        Layout layout{ .orientation = Layout::Orientation::None };

        // inner and outer margins of the panel and the control it contains
        Margins inner_margin = {
            .top    = 0,
            .bottom = 0,
            .left   = 0,
            .right  = 0,
        };

        Margins outer_margin = {
            .top    = 0,
            .bottom = 0,
            .left   = 0,
            .right  = 0,
        };
    };
}
