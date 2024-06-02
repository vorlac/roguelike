
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>

#include "ds/dims.hpp"
#include "ui/layouts/grid_layout.hpp"
#include "ui/widgets/scroll_dialog.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    ds::dims<f32> GridLayout::computed_size(nvg::Context* nvg_context, const Widget* widget) const
    {
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        ds::dims pref_size{
            (2.0f * m_margin) + std::accumulate(grid[0].begin(), grid[0].end(), 0.0f) +
                (std::max(static_cast<f32>(grid[0].size()) - 1.0f, 0.0f) * m_spacing.x),
            (2.0f * m_margin) + std::accumulate(grid[1].begin(), grid[1].end(), 0.0f) +
                std::max(static_cast<f32>(grid[1].size()) - 1.0f, 0.0f) * m_spacing.y,
        };

        const ScrollableDialog* dialog{ dynamic_cast<const ScrollableDialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            pref_size.height += dialog->header_height() - (m_margin / 2.0f);

        return pref_size;
    }

    void GridLayout::compute_layout(nvg::Context*, const Widget* widget,
                                    std::array<std::vector<f32>, 2>& grid) const
    {
        const u32 axis1{ static_cast<u32>(m_orientation) };
        const u32 axis2{ (axis1 + 1) % 2 };
        const u64 num_children{ widget->child_count() };

        i32 visible_children{ 0 };
        for (auto w : widget->children())
            visible_children += w->visible() ? 1 : 0;

        const ds::dims dim{
            m_resolution,
            (static_cast<f32>(visible_children) + m_resolution - 1.0f) / m_resolution,
        };

        grid[axis1].clear();
        grid[axis1].resize(static_cast<u32>(dim.width), 0.0f);
        grid[axis2].clear();
        grid[axis2].resize(static_cast<u32>(dim.height), 0.0f);

        const auto dim_axis2{ static_cast<i32>(axis2 == Axis::Horizontal ? dim.width : dim.height) };

        u64 child{ 0 };
        for (i32 i2 = 0; i2 < dim_axis2; i2++) {
            const i32 dim_axis1{ static_cast<i32>(axis1 == Axis::Horizontal ? dim.width
                                                                            : dim.height) };

            for (i32 i1 = 0; i1 < dim_axis1; i1++) {
                const Widget* w{ nullptr };

                do {
                    if (child >= num_children)
                        return;

                    w = widget->children()[child++];
                }
                while (!w->visible());

                ds::dims ps{ w->preferred_size() };
                ds::dims fs{ w->fixed_size() };

                ds::dims target_size{
                    std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                    std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height
                                                                               : ps.height,
                };

                auto& target_size_axis1{ axis1 == Axis::Horizontal ? target_size.width
                                                                   : target_size.height };
                auto& target_size_axis2{ axis2 == Axis::Horizontal ? target_size.width
                                                                   : target_size.height };

                grid[axis1][i1] = std::max(grid[axis1][i1], target_size_axis1);
                grid[axis2][i2] = std::max(grid[axis2][i2], target_size_axis2);
            }
        }
    }

    void GridLayout::apply_layout(nvg::Context* nvg_context, const Widget* widget) const
    {
        ds::dims fs_w{ widget->fixed_size() };
        ds::dims container_size{
            std::fabs(fs_w.width) > std::numeric_limits<f32>::epsilon() ? fs_w.width
                                                                        : widget->width(),
            std::fabs(fs_w.height) > std::numeric_limits<f32>::epsilon() ? fs_w.height
                                                                         : widget->height(),
        };
        // Compute minimum row / column sizes
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        const std::array dim{
            static_cast<f32>(grid[std::to_underlying(Axis::Horizontal)].size()),
            static_cast<f32>(grid[std::to_underlying(Axis::Vertical)].size()),
        };

        ds::dims extra{ 0.0f, 0.0f };
        const auto dialog{ dynamic_cast<const ScrollableDialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            extra.height += dialog->header_height() - (m_margin / 2.0f);

        // Strech to size provided by widget
        for (const auto cur_axis : { Axis::Horizontal, Axis::Vertical }) {
            f32 grid_size{ (2.0f * m_margin) +
                           (cur_axis == Axis::Horizontal ? extra.width : extra.height) };

            const i32 axis_idx{ std::to_underlying(cur_axis) };
            for (const auto& s : grid[axis_idx]) {
                grid_size += s;
                if (1.0f + static_cast<f32>(axis_idx) < dim[axis_idx])
                    grid_size += this->spacing(cur_axis);
            }

            const f32& axis_size{ cur_axis == Axis::Horizontal ? container_size.width
                                                               : container_size.height };
            if (grid_size < axis_size) {
                // Re-distribute remaining space evenly
                const f32 gap{ axis_size - grid_size };
                const f32 g{ gap / dim[axis_idx] };

                f32 rest{ gap - g * dim[axis_idx] };
                for (i32 i = 0; static_cast<f32>(i) < dim[axis_idx]; ++i)
                    grid[axis_idx][i] += g;
                for (i32 i = 0; rest > 0 && static_cast<f32>(i) < dim[axis_idx]; --rest, ++i)
                    grid[axis_idx][i] += 1.0f;
            }
        }

        const u32 num_children{ static_cast<u32>(widget->child_count()) };
        const u32 axis1{ static_cast<u32>(m_orientation) };
        const u32 axis2{ (axis1 + 1) % 2 };

        ds::dims<i32> start_offset{ extra + m_margin };
        ds::point<i32> start{ start_offset.width, start_offset.height };
        ds::point<i32> pos{ start };

        i32& axis1_pos{ axis1 == Alignment::Horizontal ? pos.x : pos.y };
        i32& axis2_pos{ axis2 == Alignment::Horizontal ? pos.x : pos.y };

        u32 child_idx{ 0 };
        for (u32 i2 = 0; i2 < static_cast<u32>(dim[axis2]); i2++) {
            const i32& s{ m_orientation == Alignment::Horizontal ? start.x : start.y };
            i32& p{ m_orientation == Alignment::Horizontal ? pos.x : pos.y };

            p = s;
            for (u32 i1 = 0; i1 < static_cast<u32>(dim[axis1]); i1++) {
                Widget* child{ nullptr };
                do {
                    if (child_idx >= num_children)
                        return;

                    child = widget->children()[child_idx++];
                }
                while (!child->visible());

                const ds::dims<f32> ps{ child->preferred_size() };
                ds::dims<f32> fs{ child->fixed_size() };
                ds::dims<f32> target_size{
                    std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                    std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height
                                                                               : ps.height,
                };

                ds::point<f32> item_pos{
                    static_cast<f32>(pos.x),
                    static_cast<f32>(pos.y),
                };

                for (u32 j = 0; j < 2; j++) {
                    const u32 axis_idx{ (axis1 + j) % 2 };
                    const u32 item_idx{ j == 0 ? i1 : i2 };
                    const Placement_OldAlignment align{ this->alignment(
                        static_cast<Axis>(axis_idx), static_cast<u32>(item_idx)) };

                    f32& item_axis_pos{ (axis_idx == Axis::Horizontal ? item_pos.x : item_pos.y) };
                    f32& fs_axis_size{ axis_idx == Alignment::Horizontal ? fs.width : fs.height };
                    f32& target_axis_size{ axis_idx == Alignment::Horizontal ? target_size.width
                                                                             : target_size.height };
                    switch (align) {
                        case Placement_OldAlignment::Minimum:
                            break;
                        case Placement_OldAlignment::Center:
                            item_axis_pos += (grid[axis_idx][item_idx] - target_axis_size) / 2.0f;
                            break;
                        case Placement_OldAlignment::Maximum:
                            item_axis_pos += grid[axis_idx][item_idx] - target_axis_size;
                            break;
                        case Placement_OldAlignment::Fill:
                            target_axis_size = (fs_axis_size != 0.0f) ? fs_axis_size
                                                                      : grid[axis_idx][item_idx];
                            break;
                        case Placement_OldAlignment::None:
                            debug_assert(false);
                            break;
                    }
                }

                child->set_position(item_pos);
                child->set_size(target_size);
                child->perform_layout();

                axis1_pos += static_cast<i32>(
                    grid[axis1][i1] + this->spacing(static_cast<Axis>(axis1)));
            }

            axis2_pos += static_cast<i32>(grid[axis2][i2] + this->spacing(static_cast<Axis>(axis2)));
        }
    }

    Alignment GridLayout::orientation() const
    {
        return m_orientation;
    }

    void GridLayout::set_orientation(const Alignment orientation)
    {
        m_orientation = orientation;
    }

    f32 GridLayout::resolution() const
    {
        return m_resolution;
    }

    void GridLayout::set_resolution(const f32 resolution)
    {
        m_resolution = resolution;
    }

    f32 GridLayout::spacing(const Axis axis) const
    {
        switch (axis) {
            case Axis::Horizontal:
                return m_spacing.x;
            case Axis::Vertical:
                return m_spacing.y;
            default:
                debug_assert(false, "invalid axis value");
                return -1;
        }
    }

    void GridLayout::set_spacing(const Axis axis, const f32 spacing)
    {
        switch (axis) {
            case Axis::Horizontal:
                m_spacing.x = spacing;
                break;
            case Axis::Vertical:
                m_spacing.y = spacing;
                break;
            default:
                debug_assert(false, "invalid axis value");
                break;
        }
    }

    void GridLayout::set_spacing(const f32 spacing)
    {
        m_spacing = ds::vector2{ spacing, spacing };
    }

    f32 GridLayout::margin() const
    {
        return m_margin;
    }

    void GridLayout::set_margin(f32 margin)
    {
        m_margin = margin;
    }

    Placement_OldAlignment GridLayout::alignment(Axis axis, i32 item) const
    {
        i32 axis_idx = std::to_underlying(axis);
        if (item < static_cast<i32>(m_alignment[axis_idx].size()))
            return m_alignment[axis_idx][item];
        else
            return m_default_alignment[axis_idx];
    }

    void GridLayout::set_col_alignment(Placement_OldAlignment value)
    {
        m_default_alignment[std::to_underlying(Axis::Horizontal)] = value;
    }

    void GridLayout::set_row_alignment(Placement_OldAlignment value)
    {
        m_default_alignment[std::to_underlying(Axis::Vertical)] = value;
    }

    void GridLayout::set_col_alignment(const std::vector<Placement_OldAlignment>& value)
    {
        m_alignment[std::to_underlying(Axis::Horizontal)] = value;
    }

    void GridLayout::set_row_alignment(const std::vector<Placement_OldAlignment>& value)
    {
        m_alignment[std::to_underlying(Axis::Vertical)] = value;
    }

}
