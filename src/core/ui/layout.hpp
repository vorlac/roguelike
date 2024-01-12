#pragma once

#include <unordered_map>
#include <vector>

#include <nanovg.h>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/vector2d.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    enum class alignment : i8_fast {
        Unknown = -1,  // Invalid / uninitialized alignment
        Minimum = 0,   // Take only as much space as is required.
        Middle,        // Center align.
        Maximum,       // Take as much space as is allowed.
        Fill           // Fill according to preferred sizes.
    };

    enum class orientation : i8_fast {
        Unknown = -1,    // Invalid / uninitialized orientation
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    enum axis : i8_fast {
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    struct Anchor
    {
    public:
        Anchor() = default;

        Anchor(i32 x, i32 y, ui::alignment horiz = ui::alignment::Fill,
               ui::alignment vert = ui::alignment::Fill)
            : grid_pos{ static_cast<u8>(x), static_cast<u8>(y) }
            , cell_size{ 1, 1 }
            , align{ horiz, vert }
        {
        }

        Anchor(u32 grid_x, u32 grid_y, u32 cell_span_width, u32 cell_span_height,
               ui::alignment horizontal_alignment = ui::alignment::Fill,
               ui::alignment vertical_alignment = ui::alignment::Fill)
            : grid_pos{ grid_x, grid_y }
            , cell_size{ cell_span_width, cell_span_height }
            , align{ horizontal_alignment, vertical_alignment }
        {
        }

        operator std::string() const
        {
            return fmt::format("Format[pos=({}), size=({}), align=(h:{}, v:{})]", grid_pos,
                               cell_size, static_cast<i32>(align[axis::Horizontal]),
                               static_cast<i32>(align[axis::Vertical]));
        }

    public:
        ds::point<u32> grid_pos{ 0, 0 };
        ds::dims<u32> cell_size{ 0, 0 };
        std::array<ui::alignment, 2> align{};
    };
}

namespace rl::ui {
    class Widget;

    /// @brief
    ///     The common layout interface
    class layout : public ds::refcounted
    {
    public:
        template <typename T>
        struct Spacing
        {
            T horizontal{};
            T vertical{};
        };

    public:
        /// @brief
        ///     Performs applies all layout computations for the given widget.
        ///
        /// @param  nvc
        ///     The NanoVG context being used for drawing.
        /// @param  w
        ///     The Widget whose child widgets will be positioned by the layout class.
        virtual void perform_layout(NVGcontext* nvc, ui::Widget* w) const = 0;

        /// @brief
        ///     Compute the preferred size for a given layout and widget
        ///
        /// @param  nvc
        ///     The NanoVG context being used for drawing.
        /// @param  w
        ///     Widget, whose preferred size should be computed.
        ///
        /// @returns
        ///     A ds::dims{i32}
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::Widget* w) const = 0;
    };

    /// @brief
    ///     Box Layout is a simple layout that supports horizontal and vertical
    ///     orientation.
    ///
    ///     Aside form defining the ui::layout interface for sizing and
    ///     performing the layout, a box_layout only handles basic orientation,
    ///     margins and spacing.
    class BoxLayout : public layout
    {
    public:
        BoxLayout(ui::orientation orientation,                      // horizontal or vertical
                  ui::alignment alignment = ui::alignment::Middle,  // min, middle, max, full
                  i32 margin = 0,                                   // the widget margins
                  i32 spacing = 0);                                 // the widget spacing

        i32 margin() const;
        i32 spacing() const;
        ui::alignment alignment() const;
        ui::orientation orientation() const;

        void set_margin(i32 margin);
        void set_spacing(i32 spacing);
        void set_orientation(ui::orientation orientation);
        void set_alignment(ui::alignment alignment);

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::Widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::Widget* w) const override;

    protected:
        i32 m_margin{ 0 };
        i32 m_spacing{ 0 };
        ui::orientation m_orientation{};
        ui::alignment m_alignment{};
    };

    /// Group Layout
    ///
    /// @brief
    ///     Aside form definint the ui::layout interface for sizing and performing the layout, a
    ///     box_layout only handles basic orientation, margins and spacing.
    class GroupLayout : public layout
    {
    public:
        GroupLayout(i32 margin = 15, i32 spacing = 6, i32 group_spacing = 14, i32 group_indent = 20)
            : m_margin{ margin }
            , m_spacing{ spacing }
            , m_group_spacing{ group_spacing }
            , m_group_indent{ group_indent }
        {
        }

        i32 margin() const;
        i32 spacing() const;
        i32 group_indent() const;
        i32 group_spacing() const;

        void set_margin(i32 margin);
        void set_spacing(i32 spacing);
        void set_group_indent(i32 group_indent);
        void set_group_spacing(i32 group_spacing);

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::Widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::Widget* w) const override;

    protected:
        i32 m_margin{ 15 };
        i32 m_spacing{ 0 };
        i32 m_group_spacing{ 0 };
        i32 m_group_indent{ 0 };
    };

    class GridLayout : public layout
    {
    public:
        GridLayout(ui::orientation orientation = ui::orientation::Horizontal,  //
                   i32 resolution = 2,                                         //
                   ui::alignment alignment = ui::alignment::Middle,            //
                   i32 margin = 0,                                             //
                   i32 spacing = 0)                                            //
            : m_margin{ margin }
            , m_resolution{ resolution }
            , m_spacing{ spacing, spacing }
            , m_orientation{ orientation }
            , m_default_alignment{ alignment, alignment }
        {
        }

        i32 margin() const;
        i32 resolution() const;
        i32 spacing(ui::axis axis) const;
        ui::orientation orientation() const;
        ui::alignment alignment(i32 axis, i32 item) const;

        void set_margin(i32 margin);
        void set_resolution(i32 resolution);
        void set_spacing(i32 spacing);
        void set_spacing(ui::axis axis, i32 spacing);
        void set_orientation(ui::orientation orientation);
        void set_col_alignment(ui::alignment value);
        void set_row_alignment(ui::alignment value);
        void set_col_alignment(const std::vector<ui::alignment>& value);
        void set_row_alignment(const std::vector<ui::alignment>& value);

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::Widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::Widget* w) const override;

    protected:
        void compute_layout(NVGcontext* nvc, const ui::Widget* w,
                            std::array<std::vector<i32>, 2>& grid) const;

    protected:
        i32 m_margin{ 0 };
        i32 m_resolution{ 0 };
        ds::vector2<i32> m_spacing{ 0, 0 };
        ui::orientation m_orientation{ orientation::Unknown };
        std::array<ui::alignment, 2> m_default_alignment{ { {}, {} } };
        std::array<std::vector<ui::alignment>, 2> m_alignment{ { {}, {} } };
    };

    class AdvancedGridLayout : public layout
    {
    public:
        AdvancedGridLayout(const std::vector<i32>& cols = {}, const std::vector<i32>& rows = {},
                           i32 margin = 0);

        i32 margin() const;
        u32 col_count() const;
        u32 row_count() const;
        Anchor anchor(const ui::Widget* widget) const;

        void set_margin(i32 margin);
        void append_row(i32 size, f32 stretch = 0.0f);
        void append_col(i32 size, f32 stretch = 0.0f);
        void set_row_stretch(i32 index, f32 stretch);
        void set_col_stretch(i32 index, f32 stretch);
        void set_anchor(const ui::Widget* widget, const Anchor& anchor);

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::Widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::Widget* w) const override;

    protected:
        void compute_layout(NVGcontext* nvg_context, const ui::Widget* widget,
                            std::array<std::vector<i32>, 2>& grid) const;

    protected:
        std::vector<i32> m_cols{};
        std::vector<i32> m_rows{};
        std::vector<f32> m_col_stretch{};
        std::vector<f32> m_row_stretch{};
        std::unordered_map<const ui::Widget*, Anchor> m_anchor{};
        i32 m_margin{ 0 };

    private:
        enum LayoutComputePhase {
            ComputeCellSize = 0,
            MulitCellMerge = 1,
        };
    };
}
