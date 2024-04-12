
#include <array>
#include <numeric>
#include <utility>
#include <vector>

#include "core/ui/layouts/advanced_grid_layout.hpp"
#include "core/ui/layouts/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/ui/widgets/dialog.hpp"
#include "core/ui/widgets/label.hpp"
#include "core/ui/widgets/scroll_dialog.hpp"
#include "ds/dims.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    AdvancedGridLayout::AdvancedGridLayout(const std::vector<f32>& cols,
                                           const std::vector<f32>& rows, const f32 margin)
        : m_cols(cols)
        , m_rows(rows)
        , m_margin(margin)
    {
        m_col_stretch.resize(m_cols.size(), 0.0f);
        m_row_stretch.resize(m_rows.size(), 0.0f);
    }

    ds::dims<f32> AdvancedGridLayout::computed_size(nvg::Context* nvg_context,
                                                    const Widget* widget) const
    {
        // Compute minimum row / column sizes
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        const ds::dims size{
            std::accumulate(grid[std::to_underlying(Axis::Horizontal)].begin(),
                            grid[std::to_underlying(Axis::Horizontal)].end(), 0.0f),
            std::accumulate(grid[std::to_underlying(Axis::Vertical)].begin(),
                            grid[std::to_underlying(Axis::Vertical)].end(), 0.0f),
        };

        ds::dims extra{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        const auto dialog{ dynamic_cast<const ScrollableDialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            extra.height += widget->theme()->dialog_header_height - m_margin / 2.0f;

        return size + extra;
    }

    void AdvancedGridLayout::apply_layout(nvg::Context* nvg_context, const Widget* widget) const
    {
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        grid[std::to_underlying(Axis::Horizontal)].insert(
            grid[std::to_underlying(Axis::Horizontal)].begin(), m_margin);

        const auto dialog{ dynamic_cast<const ScrollableDialog*>(widget) };
        if (dialog == nullptr || dialog->title().empty())
            grid[std::to_underlying(Axis::Vertical)].insert(
                grid[std::to_underlying(Axis::Vertical)].begin(), m_margin);
        else if (dialog != nullptr)
            grid[std::to_underlying(Axis::Vertical)].insert(
                grid[std::to_underlying(Axis::Vertical)].begin(),
                dialog->header_height() + m_margin / 2.0f);
        else
        {
            grid[std::to_underlying(Axis::Vertical)].insert(
                grid[std::to_underlying(Axis::Vertical)].begin(),
                widget->theme()->dialog_header_height + m_margin / 2.0f);
        }

        for (Axis axis : { Axis::Horizontal, Axis::Vertical })
        {
            std::vector<f32>& axis_grids{ grid[std::to_underlying(axis)] };
            for (size_t i = 1; i < axis_grids.size(); ++i)
                axis_grids[i] += axis_grids[i - 1];

            for (Widget* child : widget->children())
            {
                const ScrollableDialog* child_dialog{ dynamic_cast<ScrollableDialog*>(child) };
                if (!child->visible() || child_dialog != nullptr)
                    continue;

                if (child == nullptr)
                    continue;

                const Anchor anchor{ this->anchor(child) };
                const u32 axis_anchor_pos{ axis == Axis::Horizontal ? anchor.grid_pos.x
                                                                    : anchor.grid_pos.y };
                const u32 axis_anchor_size{ axis == Axis::Horizontal ? anchor.cell_size.width
                                                                     : anchor.cell_size.height };

                f32 item_pos{ axis_grids[axis_anchor_pos] };
                const f32 cell_size{ axis_grids[(axis_anchor_pos + axis_anchor_size)] - item_pos };

                const ds::dims widget_ps{ child->preferred_size() };
                const ds::dims widget_fs{ child->fixed_size() };
                const f32 ps{ axis == Axis::Horizontal ? widget_ps.width : widget_ps.height };
                const f32 fs{ axis == Axis::Horizontal ? widget_fs.width : widget_fs.height };

                f32 target_size{ std::fabs(fs) > std::numeric_limits<f32>::epsilon() ? fs : ps };
                switch (anchor.align[std::to_underlying(axis)])
                {
                    case Placement_OldAlignment::Minimum:
                        break;
                    case Placement_OldAlignment::Center:
                        item_pos += (cell_size - target_size) / 2.0f;
                        break;
                    case Placement_OldAlignment::Maximum:
                        item_pos += cell_size - target_size;
                        break;
                    case Placement_OldAlignment::Fill:
                        target_size = std::fabs(fs) > std::numeric_limits<f32>::epsilon()
                                        ? fs
                                        : cell_size;
                        break;
                    case Placement_OldAlignment::None:
                        assert_cond(false);
                        break;
                }

                auto pos{ child->position() };
                auto size{ child->size() };
                f32& item_axis_pos{ axis == Axis::Horizontal ? pos.x : pos.y };
                f32& item_axis_size{ axis == Axis::Horizontal ? size.width : size.height };
                item_axis_pos = item_pos;
                item_axis_size = target_size;

                child->set_position(std::move(pos));
                child->set_size(std::move(size));
                child->perform_layout();
            }
        }
    }

    void AdvancedGridLayout::compute_layout(nvg::Context*, const Widget* widget,
                                            std::array<std::vector<f32>, 2>& grid_cell_sizes) const
    {
        const ds::dims fs_w{ widget->fixed_size() };
        ds::dims container_size{
            std::fabs(fs_w.width) > std::numeric_limits<f32>::epsilon() ? fs_w.width
                                                                        : widget->width(),
            std::fabs(fs_w.height) > std::numeric_limits<f32>::epsilon() ? fs_w.height
                                                                         : widget->height(),
        };

        ds::dims extra{
            m_margin * 2.0f,
            m_margin * 2.0f,
        };

        const auto dialog{ dynamic_cast<const ScrollableDialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            extra.height += dialog->header_height() - (m_margin / 2.0f);

        container_size -= extra;
        for (const Axis axis : { Axis::Horizontal, Axis::Vertical })
        {
            std::vector<f32>& grid{ grid_cell_sizes[std::to_underlying(axis)] };
            const bool col_axis{ axis == Axis::Horizontal };
            const std::vector<f32>& sizes{ col_axis ? m_cols : m_rows };
            const std::vector<f32>& stretch{ col_axis ? m_col_stretch : m_row_stretch };

            grid = sizes;
            for (const auto phase : { ComputeCellSize, MulitCellMerge })
            {
                for (const auto& [layout_widget, widget_anchor] : m_anchor)
                {
                    const Widget* w{ layout_widget };
                    const auto anchor_window{ dynamic_cast<const ScrollableDialog*>(w) };
                    if (!w->visible() || anchor_window != nullptr)
                        continue;

                    const Anchor& anchor{ widget_anchor };
                    const u32 axis_anchor_pos{ col_axis ? anchor.grid_pos.x : anchor.grid_pos.y };
                    const u32 axis_anchor_size{ col_axis ? anchor.cell_size.width
                                                         : anchor.cell_size.height };

                    const bool compute_single_cell_sizes{ phase == ComputeCellSize };
                    const bool cell_is_single_grid_sized{ axis_anchor_size == 1 };
                    if (cell_is_single_grid_sized != compute_single_cell_sizes)
                        continue;

                    const ds::dims widget_ps{ w->preferred_size() };
                    const ds::dims widget_fs{ w->fixed_size() };

                    const f32 ps{ col_axis ? widget_ps.width : widget_ps.height };
                    const f32 fs{ col_axis ? widget_fs.width : widget_fs.height };
                    const f32 target_size{ std::fabs(fs) > std::numeric_limits<f32>::epsilon()
                                               ? fs
                                               : ps };

                    runtime_assert(axis_anchor_pos + axis_anchor_size <= grid.size(),
                                   "Advanced grid layout: widget is out of bounds");

                    f32 current_size{ 0.0f };
                    f32 total_stretch{ 0.0f };
                    for (u32 i = axis_anchor_pos; i < axis_anchor_pos + axis_anchor_size; ++i)
                    {
                        if (std::fabs(sizes[i]) < std::numeric_limits<f32>::epsilon() &&
                            axis_anchor_size == 1)
                            grid[i] = std::max(grid[i], target_size);

                        current_size += grid[i];
                        total_stretch += stretch[i];
                    }

                    if (target_size <= current_size)
                        continue;

                    runtime_assert(math::not_equal(total_stretch, 0.0f),
                                   "Advanced grid layout: no space to place widget");

                    const f32 amt{ (target_size - current_size) / total_stretch };
                    for (u32 i = axis_anchor_pos; i < axis_anchor_pos + axis_anchor_size; ++i)
                        grid[i] += std::round(amt * stretch[i]);
                }
            }

            const f32 current_size{ std::accumulate(grid.begin(), grid.end(), 0.0f) };
            const f32 total_stretch{ std::accumulate(stretch.begin(), stretch.end(), 0.0f) };
            const f32 axis_container_size{ axis == Axis::Horizontal ? container_size.width
                                                                    : container_size.height };

            if (current_size >= axis_container_size || math::equal(total_stretch, 0.0f))
                continue;

            const f32 amt{ (axis_container_size - current_size) / total_stretch };
            for (size_t i = 0; i < grid.size(); ++i)
                grid[i] += std::round(amt * stretch[i]);
        }
    }

    f32 AdvancedGridLayout::margin() const
    {
        return m_margin;
    }

    void AdvancedGridLayout::set_margin(const f32 margin)
    {
        m_margin = margin;
    }

    u32 AdvancedGridLayout::col_count() const
    {
        return static_cast<u32>(m_cols.size());
    }

    u32 AdvancedGridLayout::row_count() const
    {
        return static_cast<u32>(m_rows.size());
    }

    void AdvancedGridLayout::append_row(const f32 size, const f32 stretch)
    {
        m_rows.push_back(size);
        m_row_stretch.push_back(stretch);
    }

    void AdvancedGridLayout::append_col(const f32 size, const f32 stretch)
    {
        m_cols.push_back(size);
        m_col_stretch.push_back(stretch);
    }

    void AdvancedGridLayout::set_row_stretch(const i32 index, const f32 stretch)
    {
        m_row_stretch.at(index) = stretch;
    }

    void AdvancedGridLayout::set_col_stretch(const i32 index, const f32 stretch)
    {
        m_col_stretch.at(index) = stretch;
    }

    void AdvancedGridLayout::set_anchor(const Widget* widget, const Anchor& anchor)
    {
        m_anchor[widget] = anchor;
    }

    Anchor AdvancedGridLayout::anchor(const Widget* widget) const
    {
        const auto it{ m_anchor.find(widget) };
        runtime_assert(it != m_anchor.end(), "Widget was not registered with the grid layout!");
        return it->second;
    }
}
