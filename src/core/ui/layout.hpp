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

    enum class alignment : u8_fast {
        // Take only as much space as is required.
        Minimum = 0,
        // Center align.
        Middle,
        // Take as much space as is allowed.
        Maximum,
        // Fill according to preferred sizes.
        Fill
    };

    enum class orientation : u8_fast {
        // Layout expands on horizontal axis.
        Horizontal = 0,
        // Layout expands on vertical axis.
        Vertical
    };

    class layout : public ds::refcounted
    {
    public:
        virtual void perform_layout(NVGcontext* nvg_context, ui::widget* w) const = 0;
        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context, const ui::widget* w) const = 0;
    };

    class box_layout : public layout
    {
    public:
        box_layout(ui::orientation orientation, ui::alignment alignment = ui::alignment::Middle,
                   i32 margin = 0, i32 spacing = 0);

        ui::orientation orientation() const
        {
            return m_orientation;
        }

        void set_orientation(ui::orientation orientation)
        {
            m_orientation = orientation;
        }

        ui::alignment alignment() const
        {
            return m_alignment;
        }

        void set_alignment(ui::alignment alignment)
        {
            m_alignment = alignment;
        }

        i32 margin() const
        {
            return m_margin;
        }

        void set_margin(i32 margin)
        {
            m_margin = margin;
        }

        i32 spacing() const
        {
            return m_spacing;
        }

        void set_spacing(i32 spacing)
        {
            m_spacing = spacing;
        }

        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context,
                                             const ui::widget* widget) const override;
        virtual void perform_layout(NVGcontext* nv_context, ui::widget* widget) const override;

    protected:
        ui::orientation m_orientation{};
        ui::alignment m_alignment{};
        i32 m_margin{ 0 };
        i32 m_spacing{ 0 };
    };

    class group_layout : public layout
    {
    public:
        group_layout(i32 margin = 15, i32 spacing = 6, i32 group_spacing = 14, i32 group_indent = 20)
            : m_margin{ margin }
            , m_spacing{ spacing }
            , m_group_spacing{ group_spacing }
            , m_group_indent{ group_indent }
        {
        }

        i32 margin() const
        {
            return m_margin;
        }

        void set_margin(i32 margin)
        {
            m_margin = margin;
        }

        i32 spacing() const
        {
            return m_spacing;
        }

        void set_spacing(i32 spacing)
        {
            m_spacing = spacing;
        }

        i32 group_indent() const
        {
            return m_group_indent;
        }

        void set_group_indent(i32 group_indent)
        {
            m_group_indent = group_indent;
        }

        i32 group_spacing() const
        {
            return m_group_spacing;
        }

        void set_group_spacing(i32 group_spacing)
        {
            m_group_spacing = group_spacing;
        }

        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context,
                                             const ui::widget* widget) const override;
        virtual void perform_layout(NVGcontext* nvg_context, ui::widget* widget) const override;

    protected:
        i32 m_margin{};
        i32 m_spacing{};
        i32 m_group_spacing{};
        i32 m_group_indent{};
    };

    // class grid_layout : public layout
    //{
    // public:
    //     grid_layout(ui::orientation orientation = ui::orientation::Horizontal, i32 resolution =
    //     2,
    //                 ui::alignment alignment = ui::alignment::Middle, i32 margin = 0,
    //                 i32 spacing = 0)
    //         : m_orientation{ orientation }
    //         , m_resolution{ resolution }
    //         , m_margin{ margin }
    //         , m_default_alignment{ { alignment, alignment } }
    //         , m_spacing{ spacing, spacing }
    //     {
    //     }

    //    ui::orientation orientation() const
    //    {
    //        return m_orientation;
    //    }

    //    void set_orientation(ui::orientation orientation)
    //    {
    //        m_orientation = orientation;
    //    }

    //    i32 resolution() const
    //    {
    //        return m_resolution;
    //    }

    //    void set_resolution(i32 resolution)
    //    {
    //        m_resolution = resolution;
    //    }

    //    i32 spacing(i32 axis) const
    //    {
    //        return m_spacing[axis];
    //    }

    //    void set_spacing(i32 axis, i32 spacing)
    //    {
    //        m_spacing[axis] = spacing;
    //    }

    //    void set_spacing(i32 spacing)
    //    {
    //        m_spacing[0] = m_spacing[1] = spacing;
    //    }

    //    i32 margin() const
    //    {
    //        return m_margin;
    //    }

    //    void set_margin(i32 margin)
    //    {
    //        m_margin = margin;
    //    }

    //    ui::alignment alignment(i32 axis, i32 item) const
    //    {
    //        if (item < (i32)m_alignment[axis].size())
    //            return m_alignment[axis][item];
    //        else
    //            return m_default_alignment[axis];
    //    }

    //    void set_col_alignment(ui::alignment value)
    //    {
    //        m_default_alignment[0] = value;
    //    }

    //    void set_row_alignment(ui::alignment value)
    //    {
    //        m_default_alignment[1] = value;
    //    }

    //    void set_col_alignment(const std::vector<ui::alignment>& value)
    //    {
    //        m_alignment[0] = value;
    //    }

    //    void set_row_alignment(const std::vector<ui::alignment>& value)
    //    {
    //        m_alignment[1] = value;
    //    }

    //    virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context,
    //                                         const ui::widget* widget) const override;
    //    virtual void perform_layout(NVGcontext* nvg_context, ui::widget* widget) const override;

    // protected:
    //     // Compute the maximum row and column sizes
    //     void compute_layout(NVGcontext* nvg_context, const ui::widget* widget,
    //                         std::vector<i32>* grid) const;

    // protected:
    //     ui::orientation m_orientation{};
    //     ui::alignment m_default_alignment[2] = { {}, {} };
    //     std::vector<ui::alignment> m_alignment[2] = { {}, {} };
    //     i32 m_resolution{ 0 };
    //     ds::vector2<i32> m_spacing{ 0, 0 };
    //     i32 m_margin{ 0 };
    // };

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
