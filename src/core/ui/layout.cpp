#include <numeric>
#include <utility>

#include "core/ui/dialog.hpp"
#include "core/ui/gui_canvas.hpp"
#include "core/ui/label.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    using namespace vg;

    BoxLayout::BoxLayout(ui::Orientation orientation, ui::Alignment alignment, f32 margin,
                         f32 spacing)
        : m_orientation{ orientation }
        , m_alignment{ alignment }
        , m_margin{ margin }
        , m_spacing{ spacing }
    {
    }

    ds::dims<f32> BoxLayout::preferred_size(NVGcontext* nvg_context, const ui::Widget* widget) const
    {
        ds::dims<f32> size{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        f32 y_offset{ 0.0f };
        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
        {
            if (m_orientation == ui::Orientation::Vertical)
                size.height += widget->theme()->m_window_header_height - m_margin / 2.0f;
            else
                y_offset = widget->theme()->m_window_header_height;
        }

        bool first{ true };
        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (std::to_underlying(m_orientation) + 1) % 2 };

        for (auto w : widget->children())
        {
            if (!w->visible())
                continue;

            if (first)
                first = false;
            else
                size.height += m_spacing;

            ds::dims<f32> ps{ w->preferred_size(nvg_context) };
            ds::dims<f32> fs{ w->fixed_size() };
            ds::dims<f32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            size.width += target_size.width;
            size.height = std::max(size.height, target_size.height + (2.0f * m_margin));
            first = false;
        }

        return size + ds::dims<f32>{ 0.0f, y_offset };
    }

    void BoxLayout::perform_layout(NVGcontext* nvg_context, ui::Widget* widget) const
    {
        ds::dims<f32> fs_w{ widget->fixed_size() };
        ds::dims<f32> container_size{
            fs_w.width ? fs_w.width : widget->width(),
            fs_w.height ? fs_w.height : widget->height(),
        };

        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (std::to_underlying(m_orientation) + 1) % 2 };
        f32 position{ m_margin };
        f32 y_offset{ 0.0f };

        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
        {
            if (m_orientation == ui::Orientation::Vertical)
                position += widget->theme()->m_window_header_height - (m_margin / 2.0f);
            else
            {
                y_offset = widget->theme()->m_window_header_height;
                container_size.height -= y_offset;
            }
        }

        bool first = true;
        for (auto& w : widget->children())
        {
            if (!w->visible())
                continue;
            if (first)
                first = false;
            else
                position += this->m_spacing;

            ds::dims<f32> ps{ w->preferred_size(nvg_context) };
            ds::dims<f32> fs{ w->fixed_size() };

            ds::dims<f32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            ds::point<f32> pos{
                0.0f,
                y_offset,
            };

            pos.x = position;
            // pos[axis1] = position;

            switch (m_alignment)
            {
                case Alignment::Minimum:
                    pos.y += m_margin;
                    break;
                case Alignment::Center:
                    pos.y += (container_size.height - target_size.height) / 2.0f;
                    break;
                case Alignment::Maximum:
                    pos.y += container_size.height - target_size.height - m_margin * 2.0f;
                    break;
                case Alignment::Fill:
                    pos.y += m_margin;
                    target_size.height = fs.height ? fs.height
                                                   : (container_size.height - m_margin * 2.0f);
                    break;
            }

            w->set_position(pos);
            w->set_size(target_size);
            w->perform_layout(nvg_context);

            position += target_size.width;
            // position += target_size[axis1];
        }
    }

    ui::Orientation BoxLayout::orientation() const
    {
        return m_orientation;
    }

    void BoxLayout::set_orientation(ui::Orientation orientation)
    {
        m_orientation = orientation;
    }

    ui::Alignment BoxLayout::alignment() const
    {
        return m_alignment;
    }

    void BoxLayout::set_alignment(ui::Alignment alignment)
    {
        m_alignment = alignment;
    }

    f32 BoxLayout::margin() const
    {
        return m_margin;
    }

    void BoxLayout::set_margin(f32 margin)
    {
        m_margin = margin;
    }

    f32 BoxLayout::spacing() const
    {
        return m_spacing;
    }

    void BoxLayout::set_spacing(f32 spacing)
    {
        m_spacing = spacing;
    }

    //======================================================================

    ds::dims<f32> GroupLayout::preferred_size(NVGcontext* nvg_context,
                                              const ui::Widget* widget) const
    {
        f32 height{ m_margin };
        f32 width{ 2.0f * m_margin };

        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
            height += widget->theme()->m_window_header_height - m_margin / 2.0f;

        bool first = true, indent = false;
        for (auto&& c : widget->children())
        {
            if (!c->visible())
                continue;
            const ui::Label* label = dynamic_cast<const ui::Label*>(c);
            if (!first)
                height += (label == nullptr) ? this->m_spacing : m_group_spacing;
            first = false;

            ds::dims<f32> ps{ c->preferred_size(nvg_context) };
            ds::dims<f32> fs{ c->fixed_size() };
            ds::dims<f32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            bool indent_cur = indent && label == nullptr;
            height += target_size.height;
            width = std::max(
                width, target_size.width + 2.0f * m_margin + (indent_cur ? m_group_indent : 0.0f));

            if (label != nullptr)
                indent = !label->caption().empty();
        }

        height += m_margin;

        return ds::dims<f32>{
            width,
            height,
        };
    }

    void GroupLayout::perform_layout(NVGcontext* nvg_context, ui::Widget* widget) const
    {
        f32 height{ this->m_margin };
        f32 available_width{ (widget->fixed_width() ? widget->fixed_width() : widget->width()) -
                             2.0f * this->m_margin };

        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
            height += widget->theme()->m_window_header_height - this->m_margin / 2.0f;

        bool first{ true };
        bool indent{ false };
        for (auto&& c : widget->children())
        {
            if (!c->visible())
                continue;

            const ui::Label* label = dynamic_cast<const ui::Label*>(c);
            if (!first)
                height += (label == nullptr) ? m_spacing : m_group_spacing;

            first = false;
            bool indent_cur{ indent && label == nullptr };

            ds::dims<f32> fs{ c->fixed_size() };
            ds::dims<f32> ps{
                available_width - (indent_cur ? m_group_indent : 0.0f),
                c->preferred_size(nvg_context).height,
            };

            ds::dims<f32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            c->set_position({
                this->m_margin + (indent_cur ? m_group_indent : 0.0f),
                height,
            });
            c->set_size(target_size);
            c->perform_layout(nvg_context);

            height += target_size.height;
            if (label != nullptr)
                indent = !label->caption().empty();
        }
    }

    f32 GroupLayout::margin() const
    {
        return m_margin;
    }

    void GroupLayout::set_margin(f32 margin)
    {
        m_margin = margin;
    }

    f32 GroupLayout::spacing() const
    {
        return m_spacing;
    }

    void GroupLayout::set_spacing(f32 spacing)
    {
        m_spacing = spacing;
    }

    f32 GroupLayout::group_indent() const
    {
        return m_group_indent;
    }

    void GroupLayout::set_group_indent(f32 group_indent)
    {
        m_group_indent = group_indent;
    }

    f32 GroupLayout::group_spacing() const
    {
        return m_group_spacing;
    }

    void GroupLayout::set_group_spacing(f32 group_spacing)
    {
        m_group_spacing = group_spacing;
    }

    //==================================================================

    ds::dims<f32> GridLayout::preferred_size(NVGcontext* nvg_context, const ui::Widget* widget) const
    {
        /* Compute minimum row / column sizes */
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        ds::dims<f32> pref_size{
            (2.0f * m_margin) + std::accumulate(grid[0].begin(), grid[0].end(), 0.0f) +
                (std::max(i32(grid[0].size()) - 1, 0) * m_spacing.x),
            (2.0f * m_margin) + std::accumulate(grid[1].begin(), grid[1].end(), 0.0f) +
                std::max(static_cast<f32>(grid[1].size()) - 1.0f, 0.0f) * m_spacing.y,
        };

        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
            pref_size.height += widget->theme()->m_window_header_height - this->m_margin / 2;

        return pref_size;
    }

    void GridLayout::compute_layout(NVGcontext* nvg_context, const ui::Widget* widget,
                                    std::array<std::vector<f32>, 2>& grid) const
    {
        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (axis1 + 1) % 2 };

        i32 num_children{ widget->child_count() };
        i32 visible_children{ 0 };

        for (auto&& w : widget->children())
            visible_children += w->visible() ? 1 : 0;

        ds::dims<f32> dim{
            m_resolution,
            ((visible_children + m_resolution - 1.0f) / m_resolution),
        };

        grid[axis1].clear();
        grid[axis1].resize(static_cast<size_t>(dim.width), 0.0f);
        grid[axis2].clear();
        grid[axis2].resize(static_cast<size_t>(dim.height), 0.0f);

        auto& dim_axis2 = axis2 == std::to_underlying(ui::Axis::Horizontal) ? dim.width
                                                                            : dim.height;

        size_t child{ 0 };
        for (i32 i2 = 0; i2 < dim_axis2; i2++)
        {
            auto& dim_axis1 = axis1 == std::to_underlying(ui::Axis::Horizontal) ? dim.width
                                                                                : dim.height;

            for (i32 i1 = 0; i1 < dim_axis1; i1++)
            {
                ui::Widget* w{ nullptr };
                do
                {
                    if (child >= num_children)
                        return;

                    w = widget->children()[child++];
                }
                while (!w->visible());

                ds::dims<f32> ps{ w->preferred_size(nvg_context) };
                ds::dims<f32> fs{ w->fixed_size() };
                ds::dims<f32> target_size{
                    fs.width ? fs.width : ps.width,
                    fs.height ? fs.height : ps.height,
                };

                auto& target_size_axis1{ axis1 == std::to_underlying(ui::Axis::Horizontal)
                                             ? target_size.width
                                             : target_size.height };
                auto& target_size_axis2{ axis2 == std::to_underlying(ui::Axis::Horizontal)
                                             ? target_size.width
                                             : target_size.height };
                grid[axis1][i1] = std::max(grid[axis1][i1], target_size_axis1);
                grid[axis2][i2] = std::max(grid[axis2][i2], target_size_axis2);
            }
        }
    }

    void GridLayout::perform_layout(NVGcontext* nvg_context, ui::Widget* widget) const
    {
        ds::dims<f32> fs_w{ widget->fixed_size() };
        ds::dims<f32> container_size{
            fs_w.width ? fs_w.width : widget->width(),
            fs_w.height ? fs_w.height : widget->height(),
        };

        // Compute minimum row / column sizes
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        std::array<f32, 2> dim = {
            static_cast<f32>(grid[std::to_underlying(ui::Axis::Horizontal)].size()),
            static_cast<f32>(grid[std::to_underlying(ui::Axis::Vertical)].size()),
        };

        ds::dims<f32> extra{ 0.0f, 0.0f };
        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
            extra.height += widget->theme()->m_window_header_height - m_margin / 2;

        // Strech to size provided by ui::widget
        for (const auto cur_axis : { ui::Axis::Horizontal, ui::Axis::Vertical })
        {
            f32 grid_size{ 2.0f * m_margin +
                           (cur_axis == ui::Axis::Horizontal ? extra.width : extra.height) };

            i32 axis_idx{ std::to_underlying(cur_axis) };
            for (auto s : grid[axis_idx])
            {
                grid_size += s;
                if (axis_idx + 1.0f < dim[axis_idx])
                    grid_size += this->spacing(cur_axis);
            }

            f32& axis_size{ cur_axis == ui::Axis::Horizontal ? container_size.width
                                                             : container_size.height };
            if (grid_size < axis_size)
            {
                // Re-distribute remaining space evenly
                f32 gap{ axis_size - grid_size };
                f32 g{ gap / dim[axis_idx] };
                f32 rest{ gap - g * dim[axis_idx] };
                for (i32 i = 0; i < dim[axis_idx]; ++i)
                    grid[axis_idx][i] += g;
                for (i32 i = 0; rest > 0 && i < dim[axis_idx]; --rest, ++i)
                    grid[axis_idx][i] += 1.0f;
            }
        }

        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (axis1 + 1) % 2 };

        i32 num_children{ widget->child_count() };
        i32 child{ 0 };

        // TODO: check all of this logic
        ds::dims<i32> start_offset{ extra + this->m_margin };
        ds::point<i32> start{ start_offset.width, start_offset.height };
        ds::point<i32> pos{ start };

        i32& axis1_pos{ axis1 == std::to_underlying(ui::Orientation::Horizontal) ? pos.x : pos.y };
        i32& axis2_pos{ axis2 == std::to_underlying(ui::Orientation::Horizontal) ? pos.x : pos.y };

        for (i32 i2 = 0; i2 < dim[axis2]; i2++)
        {
            i32& s{ m_orientation == ui::Orientation::Horizontal ? start.x : start.y };
            i32& p{ m_orientation == ui::Orientation::Horizontal ? pos.x : pos.y };
            p = s;

            for (i32 i1 = 0; i1 < dim[axis1]; i1++)
            {
                ui::Widget* w{ nullptr };

                do
                {
                    if (child >= num_children)
                        return;

                    w = widget->children()[child++];
                }
                while (!w->visible());

                ds::dims<f32> ps{ w->preferred_size(nvg_context) };
                ds::dims<f32> fs{ w->fixed_size() };
                ds::dims<f32> target_size{
                    fs.width ? fs.width : ps.width,
                    fs.height ? fs.height : ps.height,
                };

                ds::point<i32> item_pos{ pos };
                for (i32 j = 0; j < 2; j++)
                {
                    i32 axis_idx{ (axis1 + j) % 2 };
                    i32 item_idx{ j == 0 ? i1 : i2 };

                    ui::Alignment align{ this->alignment(ui::Axis(axis_idx), item_idx) };

                    i32& item_axis_pos{ axis_idx == ui::Axis::Horizontal ? item_pos.x : item_pos.y };
                    f32& target_axis_size{ axis_idx == std::to_underlying(Orientation::Horizontal)
                                               ? target_size.width
                                               : target_size.height };
                    f32& fs_axis_size{ axis_idx == std::to_underlying(Orientation::Horizontal)
                                           ? fs.width
                                           : fs.height };

                    switch (align)
                    {
                        case Alignment::Minimum:
                            break;
                        case Alignment::Center:
                            item_axis_pos += static_cast<i32>(
                                (grid[axis_idx][item_idx] - target_axis_size) / 2.0f);
                            break;
                        case Alignment::Maximum:
                            item_axis_pos += static_cast<i32>(
                                grid[axis_idx][item_idx] - target_axis_size);
                            break;
                        case Alignment::Fill:
                            target_axis_size = fs_axis_size ? fs_axis_size
                                                            : grid[axis_idx][item_idx];
                            break;
                    }
                }

                w->set_position(item_pos);
                w->set_size(target_size);
                w->perform_layout(nvg_context);

                axis1_pos += static_cast<i32>(grid[axis1][i1] + this->spacing(Axis(axis1)));
            }
            axis2_pos += static_cast<i32>(grid[axis2][i2] + this->spacing(Axis(axis2)));
        }
    }

    ui::Orientation GridLayout::orientation() const
    {
        return m_orientation;
    }

    void GridLayout::set_orientation(ui::Orientation orientation)
    {
        m_orientation = orientation;
    }

    f32 GridLayout::resolution() const
    {
        return m_resolution;
    }

    void GridLayout::set_resolution(f32 resolution)
    {
        m_resolution = resolution;
    }

    f32 GridLayout::spacing(ui::Axis axis) const
    {
        switch (axis)
        {
            case Axis::Horizontal:
                return m_spacing.x;
            case Axis::Vertical:
                return m_spacing.y;
            default:
                runtime_assert(false, "invalid ui::axis value");
                return -1;
        }
    }

    void GridLayout::set_spacing(ui::Axis axis, f32 spacing)
    {
        switch (axis)
        {
            case Axis::Horizontal:
                m_spacing.x = spacing;
                break;
            case Axis::Vertical:
                m_spacing.y = spacing;
                break;
            default:
                runtime_assert(false, "invalid ui::axis value");
                break;
        }
    }

    void GridLayout::set_spacing(f32 spacing)
    {
        m_spacing = { spacing, spacing };
    }

    f32 GridLayout::margin() const
    {
        return m_margin;
    }

    void GridLayout::set_margin(f32 margin)
    {
        m_margin = margin;
    }

    ui::Alignment GridLayout::alignment(ui::Axis axis, i32 item) const
    {
        if (item < static_cast<i32>(m_alignment[axis].size()))
            return m_alignment[axis][item];
        else
            return m_default_alignment[axis];
    }

    void GridLayout::set_col_alignment(ui::Alignment value)
    {
        m_default_alignment[ui::Axis::Horizontal] = value;
    }

    void GridLayout::set_row_alignment(ui::Alignment value)
    {
        m_default_alignment[ui::Axis::Vertical] = value;
    }

    void GridLayout::set_col_alignment(const std::vector<ui::Alignment>& value)
    {
        m_alignment[ui::Axis::Horizontal] = value;
    }

    void GridLayout::set_row_alignment(const std::vector<ui::Alignment>& value)
    {
        m_alignment[ui::Axis::Vertical] = value;
    }

    //=======================================================================

    AdvancedGridLayout::AdvancedGridLayout(const std::vector<f32>& cols,
                                           const std::vector<f32>& rows, f32 margin)
        : m_cols(cols)
        , m_rows(rows)
        , m_margin(margin)
    {
        m_col_stretch.resize(m_cols.size(), 0.0f);
        m_row_stretch.resize(m_rows.size(), 0.0f);
    }

    ds::dims<f32> AdvancedGridLayout::preferred_size(NVGcontext* nvg_context,
                                                     const ui::Widget* widget) const
    {
        // Compute minimum row / column sizes
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        ds::dims<f32> size{
            std::accumulate(grid[ui::Axis::Horizontal].begin(), grid[ui::Axis::Horizontal].end(),
                            0.0f),
            std::accumulate(grid[ui::Axis::Vertical].begin(), grid[ui::Axis::Vertical].end(), 0.0f),
        };

        ds::dims<f32> extra{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && !gui_canvas->title().empty())
            extra.height += widget->theme()->m_window_header_height - m_margin / 2.0f;

        return size + extra;
    }

    void AdvancedGridLayout::perform_layout(NVGcontext* nvg_context, ui::Widget* widget) const
    {
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        grid[ui::Axis::Horizontal].insert(grid[ui::Axis::Horizontal].begin(), m_margin);
        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas == nullptr || gui_canvas->title().empty())
            grid[ui::Axis::Vertical].insert(grid[ui::Axis::Vertical].begin(), m_margin);
        else
        {
            grid[ui::Axis::Vertical].insert(
                grid[ui::Axis::Vertical].begin(),
                widget->theme()->m_window_header_height + m_margin / 2.0f);
        }

        for (ui::Axis axis : { ui::Axis::Horizontal, ui::Axis::Vertical })
        {
            std::vector<f32>& axis_grids{ grid[axis] };
            for (size_t i = 1; i < axis_grids.size(); ++i)
                axis_grids[i] += axis_grids[i - 1];

            for (ui::Widget* w : widget->children())
            {
                const ui::Dialog* child_window{ dynamic_cast<ui::Dialog*>(w) };
                if (!w->visible() || child_window != nullptr)
                    continue;

                if (w == nullptr)
                    continue;

                Anchor anchor{ this->anchor(w) };
                u32 axis_anchor_pos{ axis == ui::Axis::Horizontal ? anchor.grid_pos.x
                                                                  : anchor.grid_pos.y };
                u32 axis_anchor_size{ axis == ui::Axis::Horizontal ? anchor.cell_size.width
                                                                   : anchor.cell_size.height };

                f32 item_pos{ axis_grids[axis_anchor_pos] };
                f32 cell_size{ axis_grids[axis_anchor_pos + axis_anchor_size] - item_pos };

                ds::dims<f32> widget_ps{ w->preferred_size(nvg_context) };
                ds::dims<f32> widget_fs{ w->fixed_size() };

                f32 ps{ axis == ui::Axis::Horizontal ? widget_ps.width : widget_ps.height };
                f32 fs{ axis == ui::Axis::Horizontal ? widget_fs.width : widget_fs.height };
                f32 target_size{ fs ? fs : ps };

                ui::Alignment anchor_axis_alignment{ anchor.align[axis] };
                switch (anchor_axis_alignment)
                {
                    case Alignment::Minimum:
                        break;
                    case Alignment::Center:
                        item_pos += (cell_size - target_size) / 2.0f;
                        break;
                    case Alignment::Maximum:
                        item_pos += cell_size - target_size;
                        break;
                    case Alignment::Fill:
                        target_size = fs ? fs : cell_size;
                        break;
                }

                ds::point<f32> pos{ w->position() };
                ds::dims<f32> size{ w->size() };

                f32& item_axis_pos{ axis == ui::Axis::Horizontal ? pos.x : pos.y };
                f32& item_axis_size{ axis == ui::Axis::Horizontal ? size.width : size.height };

                item_axis_pos = item_pos;
                item_axis_size = target_size;

                w->set_position(pos);
                w->set_size(size);
                w->perform_layout(nvg_context);
            }
        }
    }

    void AdvancedGridLayout::compute_layout(NVGcontext* nvg_context, const ui::Widget* widget,
                                            std::array<std::vector<f32>, 2>& grid_cell_sizes) const
    {
        ds::dims<f32> fs_w{ widget->fixed_size() };
        ds::dims<f32> container_size{
            fs_w.width ? fs_w.width : widget->width(),
            fs_w.height ? fs_w.height : widget->height(),
        };

        ds::dims<f32> extra{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        const ui::UICanvas* gui_canvas{ dynamic_cast<const ui::UICanvas*>(widget) };
        if (gui_canvas != nullptr && gui_canvas->title().length() == 0)
            extra.height += widget->theme()->m_window_header_height - this->m_margin / 2;

        container_size -= extra;

        for (ui::Axis axis : { ui::Axis::Horizontal, ui::Axis::Vertical })
        {
            std::vector<f32>& grid{ grid_cell_sizes[axis] };
            const bool col_axis{ axis == ui::Axis::Horizontal };
            const std::vector<f32>& sizes{ col_axis ? m_cols : m_rows };
            const std::vector<f32>& stretch{ col_axis ? m_col_stretch : m_row_stretch };

            grid = sizes;

            for (auto phase : { ComputeCellSize, MulitCellMerge })
            {
                for (const auto& pair : m_anchor)
                {
                    const ui::Widget* w{ pair.first };
                    const ui::Dialog* anchor_window{ dynamic_cast<const ui::Dialog*>(w) };
                    if (!w->visible() || anchor_window != nullptr)
                        continue;

                    const Anchor& anchor{ pair.second };
                    u32 axis_anchor_pos{ col_axis ? anchor.grid_pos.x : anchor.grid_pos.y };
                    u32 axis_anchor_size{ col_axis ? anchor.cell_size.width
                                                   : anchor.cell_size.height };

                    const bool compute_single_cell_sizes{ phase == ComputeCellSize };
                    const bool cell_is_single_grid_sized{ axis_anchor_size == 1 };
                    if (cell_is_single_grid_sized != compute_single_cell_sizes)
                        continue;

                    ds::dims<f32> widget_ps{ w->preferred_size(nvg_context) };
                    ds::dims<f32> widget_fs{ w->fixed_size() };

                    f32 ps{ col_axis ? widget_ps.width : widget_ps.height };
                    f32 fs{ col_axis ? widget_fs.width : widget_fs.height };
                    f32 target_size{ fs ? fs : ps };

                    runtime_assert(axis_anchor_pos + axis_anchor_size <= grid.size(),
                                   "Advanced grid layout: widget is out of bounds: {}", anchor);

                    f32 current_size{ 0.0f };
                    f32 total_stretch{ 0.0f };

                    for (u32 i = axis_anchor_pos; i < axis_anchor_pos + axis_anchor_size; ++i)
                    {
                        if (sizes[i] == 0 && axis_anchor_size == 1)
                            grid[i] = std::max(grid[i], target_size);

                        current_size += grid[i];
                        total_stretch += stretch[i];
                    }

                    if (target_size <= current_size)
                        continue;

                    runtime_assert(total_stretch != 0,
                                   "Advanced grid layout: no space to place widget: {}", anchor);

                    f32 amt{ (target_size - current_size) / total_stretch };
                    for (u32 i = axis_anchor_pos; i < axis_anchor_pos + axis_anchor_size; ++i)
                        grid[i] += static_cast<i32>(std::round(amt * stretch[i]));
                }
            }

            f32 current_size{ std::accumulate(grid.begin(), grid.end(), 0.0f) };
            f32 total_stretch{ std::accumulate(stretch.begin(), stretch.end(), 0.0f) };
            f32 axis_container_size{ axis == ui::Axis::Horizontal ? container_size.width
                                                                  : container_size.height };

            if (current_size >= axis_container_size || total_stretch == 0.0f)
                continue;

            f32 amt{ (axis_container_size - current_size) / total_stretch };
            for (size_t i = 0; i < grid.size(); ++i)
                grid[i] += std::round(amt * stretch[i]);
        }
    }

    f32 AdvancedGridLayout::margin() const
    {
        return m_margin;
    }

    void AdvancedGridLayout::set_margin(f32 margin)
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

    void AdvancedGridLayout::append_row(f32 size, f32 stretch)
    {
        m_rows.push_back(size);
        m_row_stretch.push_back(stretch);
    }

    void AdvancedGridLayout::append_col(f32 size, f32 stretch)
    {
        m_cols.push_back(size);
        m_col_stretch.push_back(stretch);
    }

    void AdvancedGridLayout::set_row_stretch(i32 index, f32 stretch)
    {
        m_row_stretch.at(index) = stretch;
    }

    void AdvancedGridLayout::set_col_stretch(i32 index, f32 stretch)
    {
        m_col_stretch.at(index) = stretch;
    }

    void AdvancedGridLayout::set_anchor(const ui::Widget* widget, const Anchor& anchor)
    {
        m_anchor[widget] = anchor;
    }

    ui::Anchor AdvancedGridLayout::anchor(const ui::Widget* widget) const
    {
        auto it{ m_anchor.find(widget) };
        runtime_assert(it != m_anchor.end(), "Widget was not registered with the grid layout!");
        return it->second;
    }
}
