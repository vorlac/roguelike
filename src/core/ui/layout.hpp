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
    class widget;

    /// @brief
    ///     Values that represent layout alignments
    enum class alignment : i8_fast {
        Unknown = -1,  // Invalid / uninitialized alignment
        Minimum = 0,   // Take only as much space as is required.
        Middle,        // Center align.
        Maximum,       // Take as much space as is allowed.
        Fill           // Fill according to preferred sizes.
    };

    /// @brief
    ///     Values that represent layout orientations
    enum class orientation : i8_fast {
        Unknown = -1,    // Invalid / uninitialized orientation
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    /// @brief
    ///     Values that represent layout orientations
    enum class axis : i8_fast {
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

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
        virtual void perform_layout(NVGcontext* nvc, ui::widget* w) const = 0;

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
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::widget* w) const = 0;
    };

    /// @brief
    ///     Box Layout is a simple layout that supports horizontal and vertical
    ///     orientation.
    ///
    ///     Aside form defining the ui::layout interface for sizing and
    ///     performing the layout, a box_layout only handles basic orientation,
    ///     margins and spacing.
    class box_layout : public layout
    {
    public:
        /// @brief
        ///     Construct a new box_layout object
        ///
        /// @param  orientation
        ///     The orientation.
        /// @param  alignment
        ///     (Optional) The alignment.
        /// @param  margin
        ///     (Optional) The margin.
        /// @param  spacing
        ///     (Optional) The spacing.
        box_layout(ui::orientation orientation,                      // horizontal or vertical
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
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::widget* w) const override;

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
    class group_layout : public layout
    {
    public:
        /// @brief
        ///     Construct a new ui::group_layout
        ///
        /// @param  margin
        ///     (Optional) The margin.
        /// @param  spacing
        ///     (Optional) The spacing.
        /// @param  group_spacing
        ///     (Optional) The group spacing.
        /// @param  group_indent
        ///     (Optional) The group indent.
        group_layout(i32 margin = 15, i32 spacing = 6, i32 group_spacing = 14, i32 group_indent = 20)
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
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::widget* w) const override;

    protected:
        i32 m_margin{ 15 };
        i32 m_spacing{ 0 };
        i32 m_group_spacing{ 0 };
        i32 m_group_indent{ 0 };
    };

    class grid_layout : public layout
    {
    public:
        /// @brief
        ///     Creates a GroupLayout.
        ///
        /// @param  margin
        ///     The margin around the widgets added.
        /// @param  spacing
        ///     The spacing between widgets added.
        /// @param  group_spacing
        ///     The spacing between groups (groups are defined by each Label added).
        /// @param  group_indent
        ///     The amount to indent widgets in a group (underneath a Label).
        grid_layout(ui::orientation orientation = ui::orientation::Horizontal,  //
                    i32 resolution = 2,                                         //
                    ui::alignment alignment = ui::alignment::Middle,            //
                    i32 margin = 0,                                             //
                    i32 spacing = 0)                                            //
            : m_margin{ margin }
            , m_resolution{ resolution }
            , m_spacing{ spacing, spacing }
            , m_orientation{ orientation }
            , m_default_alignment{ { alignment }, { alignment } }
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
        virtual ds::dims<i32> preferred_size(NVGcontext* nvc, const ui::widget* w) const override;
        virtual void perform_layout(NVGcontext* nvc, ui::widget* w) const override;

    protected:
        void compute_layout(NVGcontext* nvc, const ui::widget* w,
                            std::array<std::vector<i32>, 2>& grid) const;

    protected:
        i32 m_margin{ 0 };
        i32 m_resolution{ 0 };
        ds::vector2<i32> m_spacing{ 0, 0 };
        ui::orientation m_orientation{ orientation::Unknown };
        ui::alignment m_default_alignment[2] = { {}, {} };
        std::vector<ui::alignment> m_alignment[2] = { {}, {} };
    };

    // class advanced_grid_layout : public layout
    //{
    // public:
    //     struct anchor
    //     {
    //         // The (x, y) position.
    //         u8 pos[2] = { 0 };
    //         // The (x, y) size.
    //         u8 size[2] = { 0 };
    //         // The (x, y) alignment.
    //         ui::alignment align[2] = {};

    //        anchor()
    //        {
    //        }

    //        anchor(i32 x, i32 y, ui::alignment horiz = ui::alignment::Fill,
    //               ui::alignment vert = ui::alignment::Fill)
    //        {
    //            pos[0] = (uint8_t)x;
    //            pos[1] = (uint8_t)y;
    //            size[0] = size[1] = 1;
    //            align[0] = horiz;
    //            align[1] = vert;
    //        }

    //        anchor(i32 x, i32 y, i32 w, i32 h, ui::alignment horiz = ui::alignment::Fill,
    //               ui::alignment vert = ui::alignment::Fill)
    //        {
    //            pos[0] = (u8)x;
    //            pos[1] = (uint8_t)y;
    //            size[0] = (uint8_t)w;
    //            size[1] = (uint8_t)h;
    //            align[0] = horiz;
    //            align[1] = vert;
    //        }

    //        operator std::string() const
    //        {
    //            char buf[50];
    //            snprintf(buf, 50, "Format[pos=(%i, %i), size=(%i, %i), align=(%i, %i)]", pos[0],
    //                     pos[1], size[0], size[1], (i32)align[0], (i32)align[1]);
    //            return buf;
    //        }
    //    };

    //    advanced_grid_layout(const std::vector<i32>& cols = {}, const std::vector<i32>& rows = {},
    //                         i32 margin = 0);

    //    i32 margin() const
    //    {
    //        return m_margin;
    //    }

    //    void set_margin(i32 margin)
    //    {
    //        m_margin = margin;
    //    }

    //    i32 col_count() const
    //    {
    //        return (i32)m_cols.size();
    //    }

    //    i32 row_count() const
    //    {
    //        return (i32)m_rows.size();
    //    }

    //    void append_row(i32 size, float stretch = 0.f)
    //    {
    //        m_rows.push_back(size);
    //        m_row_stretch.push_back(stretch);
    //    };

    //    void append_col(i32 size, float stretch = 0.f)
    //    {
    //        m_cols.push_back(size);
    //        m_col_stretch.push_back(stretch);
    //    };

    //    void set_row_stretch(i32 index, float stretch)
    //    {
    //        m_row_stretch.at(index) = stretch;
    //    }

    //    void set_col_stretch(i32 index, float stretch)
    //    {
    //        m_col_stretch.at(index) = stretch;
    //    }

    //    void set_anchor(const ui::widget* widget, const anchor& anchor)
    //    {
    //        m_anchor[widget] = anchor;
    //    }

    //    anchor anchor(const ui::widget* widget) const
    //    {
    //        auto it = m_anchor.find(widget);
    //        if (it == m_anchor.end())
    //            throw std::runtime_error("ui::widget was not registered with the grid layout!");
    //        return it->second;
    //    }

    //    virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context,
    //                                         const ui::widget* widget) const override;
    //    virtual void perform_layout(NVGcontext* nvg_context, ui::widget* widget) const override;

    // protected:
    //     void compute_layout(NVGcontext* nvg_context, const ui::widget* widget,
    //                         std::vector<i32>* grid) const;

    // protected:
    //     std::vector<i32> m_cols{};
    //     std::vector<i32> m_rows{};
    //     std::vector<f32> m_col_stretch{};
    //     std::vector<f32> m_row_stretch{};
    //     std::unordered_map<const ui::widget*, anchor> m_anchor{};
    //     i32 m_margin{};
    // };
}
