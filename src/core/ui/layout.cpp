#include <numeric>
#include <utility>

#include <nanovg.h>

#include "core/ui/label.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/window.hpp"
#include "ds/dims.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    box_layout::box_layout(ui::orientation orientation, ui::alignment alignment, i32 margin,
                           i32 spacing)
        : m_orientation{ orientation }
        , m_alignment{ alignment }
        , m_margin{ margin }
        , m_spacing{ spacing }
    {
    }

    ds::dims<i32> box_layout::preferred_size(NVGcontext* nvg_context, const ui::widget* widget) const
    {
        ds::dims<i32> size{
            2 * this->m_margin,
            2 * this->m_margin,
        };

        i32 y_offset{ 0 };
        const rl::Window* window = dynamic_cast<const rl::Window*>(widget);
        if (window != nullptr && !window->title().empty())
        {
            if (m_orientation == ui::orientation::Vertical)
                size.height += widget->theme()->m_window_header_height - this->m_margin / 2;
            else
                y_offset = widget->theme()->m_window_header_height;
        }

        bool first{ true };
        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (std::to_underlying(m_orientation) + 1) % 2 };

        for (auto& w : widget->children())
        {
            if (!w->visible())
                continue;

            if (first)
                first = false;
            else
            {
                // TODO: update width instead based
                // on avis/orientation?
                // size[axis1] += this->m_spacing;
                size.height += this->m_spacing;
            }

            ds::dims<i32> ps{ w->preferred_size(nvg_context) };
            ds::dims<i32> fs{ w->fixed_size() };
            ds::dims<i32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            // size[axis1] += target_size[axis1];
            size.width += target_size.width;
            // size[axis2] = std::max(size[axis2], target_size[axis2] + 2 * this->m_margin);
            size.height = std::max(size.height, target_size.height + (2 * this->m_margin));
            first = false;
        }
        return size + ds::dims<i32>(0, y_offset);
    }

    void box_layout::perform_layout(NVGcontext* nvg_context, ui::widget* widget) const
    {
        ds::dims<i32> fs_w = widget->fixed_size();
        ds::dims<i32> container_size(fs_w.width ? fs_w.width : widget->width(),
                                     fs_w.height ? fs_w.height : widget->height());

        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (std::to_underlying(m_orientation) + 1) % 2 };
        i32 position{ this->m_margin };
        i32 y_offset{ 0 };

        const rl::Window* window{ static_cast<const rl::Window*>(widget) };
        runtime_assert(window != nullptr, "failed ui::widget cast to rl::Window");
        if (window != nullptr && !window->title().empty())
        {
            if (m_orientation == ui::orientation::Vertical)
                position += widget->theme()->m_window_header_height - this->m_margin / 2;
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

            ds::dims<i32> ps{ w->preferred_size(nvg_context) };
            ds::dims<i32> fs{ w->fixed_size() };

            ds::dims<i32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            ds::point<i32> pos{
                0,
                y_offset,
            };

            pos.x = position;
            // pos[axis1] = position;

            switch (m_alignment)
            {
                case alignment::Minimum:
                    pos.y += this->m_margin;
                    break;
                case alignment::Middle:
                    pos.y += (container_size.height - target_size.height) / 2;
                    break;
                case alignment::Maximum:
                    pos.y += container_size.height - target_size.height - this->m_margin * 2;
                    break;
                case alignment::Fill:
                    pos.y += this->m_margin;
                    target_size.height = fs.height ? fs.height
                                                   : (container_size.height - this->m_margin * 2);
                    break;
            }

            w->set_position(pos);
            w->set_size(target_size);
            w->perform_layout(nvg_context);

            position += target_size.width;
            // position += target_size[axis1];
        }
    }

    ui::orientation box_layout::orientation() const
    {
        return m_orientation;
    }

    void box_layout::set_orientation(ui::orientation orientation)
    {
        m_orientation = orientation;
    }

    ui::alignment box_layout::alignment() const
    {
        return m_alignment;
    }

    void box_layout::set_alignment(ui::alignment alignment)
    {
        m_alignment = alignment;
    }

    i32 box_layout::margin() const
    {
        return m_margin;
    }

    void box_layout::set_margin(i32 margin)
    {
        m_margin = margin;
    }

    i32 box_layout::spacing() const
    {
        return m_spacing;
    }

    void box_layout::set_spacing(i32 spacing)
    {
        m_spacing = spacing;
    }

    //======================================================================

    ds::dims<i32> group_layout::preferred_size(NVGcontext* nvg_context,
                                               const ui::widget* widget) const
    {
        i32 height = this->m_margin, width = 2 * this->m_margin;

        const rl::Window* window{ dynamic_cast<const rl::Window*>(widget) };
        if (window != nullptr && !window->title().empty())
            height += widget->theme()->m_window_header_height - this->m_margin / 2;

        bool first = true, indent = false;
        for (auto&& c : widget->children())
        {
            if (!c->visible())
                continue;
            const ui::label* label = dynamic_cast<const ui::label*>(c);
            if (!first)
                height += (label == nullptr) ? this->m_spacing : m_group_spacing;
            first = false;

            ds::dims<i32> ps{ c->preferred_size(nvg_context) };
            ds::dims<i32> fs{ c->fixed_size() };
            ds::dims<i32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            bool indent_cur = indent && label == nullptr;
            height += target_size.height;
            width = std::max(width, target_size.width + 2 * this->m_margin +
                                        (indent_cur ? this->m_group_indent : 0));

            if (label)
                indent = !label->caption().empty();
        }
        height += this->m_margin;
        return ds::dims<i32>(width, height);
    }

    void group_layout::perform_layout(NVGcontext* nvg_context, ui::widget* widget) const
    {
        i32 height{ this->m_margin };
        i32 available_width{ (widget->fixed_width() ? widget->fixed_width() : widget->width()) -
                             2 * this->m_margin };

        const rl::Window* window = dynamic_cast<const rl::Window*>(widget);
        if (window != nullptr && !window->title().empty())
            height += widget->theme()->m_window_header_height - this->m_margin / 2;

        bool first{ true };
        bool indent{ false };
        for (auto&& c : widget->children())
        {
            if (!c->visible())
                continue;

            const ui::label* label = dynamic_cast<const ui::label*>(c);
            if (!first)
                height += (label == nullptr) ? this->m_spacing : m_group_spacing;

            first = false;
            bool indent_cur{ indent && label == nullptr };

            ds::dims<i32> fs{ c->fixed_size() };
            ds::dims<i32> ps{
                available_width - (indent_cur ? this->m_group_indent : 0),
                c->preferred_size(nvg_context).height,
            };

            ds::dims<i32> target_size{
                fs.width ? fs.width : ps.width,
                fs.height ? fs.height : ps.height,
            };

            c->set_position({
                this->m_margin + (indent_cur ? this->m_group_indent : 0),
                height,
            });
            c->set_size(target_size);
            c->perform_layout(nvg_context);

            height += target_size.height;
            if (label != nullptr)
                indent = !label->caption().empty();
        }
    }

    i32 group_layout::margin() const
    {
        return m_margin;
    }

    void group_layout::set_margin(i32 margin)
    {
        m_margin = margin;
    }

    i32 group_layout::spacing() const
    {
        return m_spacing;
    }

    void group_layout::set_spacing(i32 spacing)
    {
        m_spacing = spacing;
    }

    i32 group_layout::group_indent() const
    {
        return m_group_indent;
    }

    void group_layout::set_group_indent(i32 group_indent)
    {
        m_group_indent = group_indent;
    }

    i32 group_layout::group_spacing() const
    {
        return m_group_spacing;
    }

    void group_layout::set_group_spacing(i32 group_spacing)
    {
        m_group_spacing = group_spacing;
    }

    //==================================================================

    ds::dims<i32> grid_layout::preferred_size(NVGcontext* nvg_context,
                                              const ui::widget* widget) const
    {
        /* Compute minimum row / column sizes */
        std::array<std::vector<i32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        ds::dims<i32> pref_size{
            (2 * m_margin) + std::accumulate(grid[0].begin(), grid[0].end(), 0) +
                (std::max(i32(grid[0].size()) - 1, 0) * m_spacing.x),
            (2 * m_margin) + std::accumulate(grid[1].begin(), grid[1].end(), 0) +
                std::max(static_cast<i32>(grid[1].size()) - 1, 0) * m_spacing.y,
        };

        const rl::Window* window{ static_cast<const rl::Window*>(widget) };
        if (window != nullptr && !window->title().empty())
            pref_size.height += widget->theme()->m_window_header_height - this->m_margin / 2;

        return pref_size;
    }

    void grid_layout::compute_layout(NVGcontext* nvg_context, const ui::widget* widget,
                                     std::array<std::vector<i32>, 2>& grid) const
    {
        i32 axis1{ std::to_underlying(m_orientation) };
        i32 axis2{ (axis1 + 1) % 2 };

        i32 num_children{ widget->child_count() };
        i32 visible_children{ 0 };

        for (auto&& w : widget->children())
            visible_children += w->visible() ? 1 : 0;

        ds::dims<i32> dim{
            m_resolution,
            ((visible_children + m_resolution - 1) / m_resolution),
        };

        grid[axis1].clear();
        grid[axis1].resize(dim.width, 0);
        grid[axis2].clear();
        grid[axis2].resize(dim.height, 0);

        auto& dim_axis2 = axis2 == std::to_underlying(ui::axis::Horizontal) ? dim.width
                                                                            : dim.height;

        size_t child{ 0 };
        for (i32 i2 = 0; i2 < dim_axis2; i2++)
        {
            auto& dim_axis1 = axis1 == std::to_underlying(ui::axis::Horizontal) ? dim.width
                                                                                : dim.height;

            for (i32 i1 = 0; i1 < dim_axis1; i1++)
            {
                ui::widget* w = nullptr;
                do
                {
                    if (child >= num_children)
                        return;
                    w = widget->children()[child++];
                }
                while (!w->visible());

                ds::dims<i32> ps{ w->preferred_size(nvg_context) };
                ds::dims<i32> fs{ w->fixed_size() };
                ds::dims<i32> target_size{
                    fs.width ? fs.width : ps.width,
                    fs.height ? fs.height : ps.height,
                };

                auto& target_size_axis1{ axis1 == std::to_underlying(ui::axis::Horizontal)
                                             ? target_size.width
                                             : target_size.height };
                auto& target_size_axis2{ axis2 == std::to_underlying(ui::axis::Horizontal)
                                             ? target_size.width
                                             : target_size.height };
                grid[axis1][i1] = std::max(grid[axis1][i1], target_size_axis1);
                grid[axis2][i2] = std::max(grid[axis2][i2], target_size_axis2);
            }
        }
    }

    void grid_layout::perform_layout(NVGcontext* nvg_context, ui::widget* widget) const
    {
        ds::dims<i32> fs_w{ widget->fixed_size() };
        ds::dims<i32> container_size{
            fs_w.width ? fs_w.width : widget->width(),
            fs_w.height ? fs_w.height : widget->height(),
        };

        // Compute minimum row / column sizes
        std::array<std::vector<i32>, 2> grid{ { {}, {} } };
        compute_layout(nvg_context, widget, grid);
        std::array<i32, 2> dim = {
            static_cast<i32>(grid[std::to_underlying(axis::Horizontal)].size()),
            static_cast<i32>(grid[std::to_underlying(axis::Vertical)].size()),
        };

        ds::dims<i32> extra{ 0, 0 };
        const rl::Window* window{ static_cast<const rl::Window*>(widget) };
        if (window != nullptr && !window->title().empty())
            extra.height += widget->theme()->m_window_header_height - this->m_margin / 2;

        // Strech to size provided by ui::widget
        for (const auto cur_axis : { ui::axis::Horizontal, ui::axis::Vertical })
        {
            i32 grid_size{ 2 * this->m_margin +
                           (cur_axis == axis::Horizontal ? extra.width : extra.height) };

            i32 axis_idx{ std::to_underlying(cur_axis) };
            for (i32 s : grid[axis_idx])
            {
                grid_size += s;
                if (axis_idx + 1 < dim[axis_idx])
                    grid_size += this->spacing(cur_axis);
            }

            i32& axis_size{ cur_axis == axis::Horizontal ? container_size.width
                                                         : container_size.height };
            if (grid_size < axis_size)
            {
                // Re-distribute remaining space evenly
                i32 gap{ axis_size - grid_size };
                i32 g{ gap / dim[axis_idx] };
                i32 rest{ gap - g * dim[axis_idx] };
                for (i32 i = 0; i < dim[axis_idx]; ++i)
                    grid[axis_idx][i] += g;
                for (i32 i = 0; rest > 0 && i < dim[axis_idx]; --rest, ++i)
                    grid[axis_idx][i] += 1;
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

        i32& axis1_pos{ axis1 == std::to_underlying(ui::orientation::Horizontal) ? pos.x : pos.y };
        i32& axis2_pos{ axis2 == std::to_underlying(ui::orientation::Horizontal) ? pos.x : pos.y };

        for (i32 i2 = 0; i2 < dim[axis2]; i2++)
        {
            i32& s{ m_orientation == ui::orientation::Horizontal ? start.x : start.y };
            i32& p{ m_orientation == ui::orientation::Horizontal ? pos.x : pos.y };
            p = s;

            for (i32 i1 = 0; i1 < dim[axis1]; i1++)
            {
                ui::widget* w{ nullptr };

                do
                {
                    if (child >= num_children)
                        return;

                    w = widget->children()[child++];
                }
                while (!w->visible());

                ds::dims<i32> ps{ w->preferred_size(nvg_context) };
                ds::dims<i32> fs{ w->fixed_size() };
                ds::dims<i32> target_size{
                    fs.width ? fs.width : ps.width,
                    fs.height ? fs.height : ps.height,
                };

                ds::point<i32> item_pos{ pos };
                for (i32 j = 0; j < 2; j++)
                {
                    i32 axis_idx{ (axis1 + j) % 2 };
                    i32 item_idx{ j == 0 ? i1 : i2 };

                    ui::alignment align{ this->alignment(axis_idx, item_idx) };

                    i32& item_axis_pos{
                        axis_idx == std::to_underlying(ui::axis::Horizontal) ? item_pos.x
                                                                             : item_pos.y,
                    };
                    i32& target_axis_size{
                        axis_idx == std::to_underlying(ui::orientation::Horizontal)
                            ? target_size.width
                            : target_size.height,
                    };
                    i32& fs_axis_size{
                        axis_idx == std::to_underlying(ui::orientation::Horizontal) ? fs.width
                                                                                    : fs.height,
                    };

                    switch (align)
                    {
                        case ui::alignment::Minimum:
                            break;
                        case ui::alignment::Middle:
                            item_axis_pos += (grid[axis_idx][item_idx] - target_axis_size) / 2;
                            break;
                        case ui::alignment::Maximum:
                            item_axis_pos += grid[axis_idx][item_idx] - target_axis_size;
                            break;
                        case ui::alignment::Fill:
                            target_axis_size = fs_axis_size ? fs_axis_size
                                                            : grid[axis_idx][item_idx];
                            break;
                    }
                }
                w->set_position(item_pos);
                w->set_size(target_size);
                w->perform_layout(nvg_context);

                axis1_pos += grid[axis1][i1] + this->spacing(ui::axis(axis1));
            }
            axis2_pos += grid[axis2][i2] + this->spacing(ui::axis(axis2));
        }
    }

    ui::orientation grid_layout::orientation() const
    {
        return m_orientation;
    }

    void grid_layout::set_orientation(ui::orientation orientation)
    {
        m_orientation = orientation;
    }

    i32 grid_layout::resolution() const
    {
        return m_resolution;
    }

    void grid_layout::set_resolution(i32 resolution)
    {
        m_resolution = resolution;
    }

    i32 grid_layout::spacing(ui::axis axis) const
    {
        switch (axis)
        {
            case ui::axis::Horizontal:
                return m_spacing.x;
            case ui::axis::Vertical:
                return m_spacing.y;
            default:
                runtime_assert(false, "invalid ui::axis value");
                return -1;
        }
    }

    void grid_layout::set_spacing(ui::axis axis, i32 spacing)
    {
        switch (axis)
        {
            case ui::axis::Horizontal:
                m_spacing.x = spacing;
                break;
            case ui::axis::Vertical:
                m_spacing.y = spacing;
                break;
        }
    }

    void grid_layout::set_spacing(i32 spacing)
    {
        m_spacing = { spacing, spacing };
    }

    i32 grid_layout::margin() const
    {
        return m_margin;
    }

    void grid_layout::set_margin(i32 margin)
    {
        m_margin = margin;
    }

    ui::alignment grid_layout::alignment(i32 axis, i32 item) const
    {
        if (item < (i32)m_alignment[axis].size())
            return m_alignment[axis][item];
        else
            return m_default_alignment[axis];
    }

    void grid_layout::set_col_alignment(ui::alignment value)
    {
        m_default_alignment[0] = value;
    }

    void grid_layout::set_row_alignment(ui::alignment value)
    {
        m_default_alignment[1] = value;
    }

    void grid_layout::set_col_alignment(const std::vector<ui::alignment>& value)
    {
        m_alignment[0] = value;
    }

    void grid_layout::set_row_alignment(const std::vector<ui::alignment>& value)
    {
        m_alignment[1] = value;
    }

    //=======================================================================

    // advanced_grid_layout::advanced_grid_layout(const std::vector<i32>& cols,
    //                                            const std::vector<i32>& rows, i32 margin)
    //     : m_cols(cols)
    //     , m_rows(rows)
    //     , this->m_margin(margin)
    //{
    //     m_col_stretch.resize(m_cols.size(), 0);
    //     m_row_stretch.resize(m_rows.size(), 0);
    // }

    // ds::dims<i32> advanced_grid_layout::preferred_size(NVGcontext* nvg_context,
    //                                                    const ui::widget* ui::widget) const
    //{
    //     // Compute minimum row / column sizes
    //     std::vector<i32> grid[2];
    //     compute_layout(nvg_context, ui::widget, grid);

    //    ds::dims<i32> size(std::accumulate(grid[0].begin(), grid[0].end(), 0),
    //                       std::accumulate(grid[1].begin(), grid[1].end(), 0));

    //    ds::dims<i32> extra(2 * this->m_margin);
    //    const rl::Window* window = dynamic_cast<const rl::Window*>(ui::widget);
    //    if (window && !window->title().empty())
    //        extra[1] += ui::widget->theme()->m_window_header_height - this->m_margin / 2;

    //    return size + extra;
    //}

    // void advanced_grid_layout::perform_layout(NVGcontext* nvg_context, ui::widget* ui::widget)
    // const
    //{
    //     std::vector<i32> grid[2];
    //     compute_layout(nvg_context, ui::widget, grid);

    //    grid[0].insert(grid[0].begin(), this->m_margin);
    //    const rl::Window* window = dynamic_cast<const rl::Window*>(ui::widget);
    //    if (window && !window->title().empty())
    //        grid[1].insert(grid[1].begin(),
    //                       ui::widget->theme()->m_window_header_height + this->m_margin / 2);
    //    else
    //        grid[1].insert(grid[1].begin(), this->m_margin);

    //    for (i32 axis = 0; axis < 2; ++axis)
    //    {
    //        for (size_t i = 1; i < grid[axis].size(); ++i)
    //            grid[axis][i] += grid[axis][i - 1];

    //        for (ui::widget* w : ui::widget->children())
    //        {
    //            if (!w->visible() || dynamic_cast<const rl::Window*>(w) != nullptr)
    //                continue;
    //            Anchor anchor = this->anchor(w);

    //            i32 item_pos = grid[axis][anchor.pos[axis]];
    //            i32 cell_size = grid[axis][anchor.pos[axis] + anchor.size[axis]] - item_pos;
    //            i32 ps = w->preferred_size(nvg_context)[axis], fs = w->fixed_size()[axis];
    //            i32 target_size = fs ? fs : ps;

    //            switch (anchor.align[axis])
    //            {
    //                case ui::alignment::Minimum:
    //                    break;
    //                case ui::alignment::Middle:
    //                    item_pos += (cell_size - target_size) / 2;
    //                    break;
    //                case ui::alignment::Maximum:
    //                    item_pos += cell_size - target_size;
    //                    break;
    //                case ui::alignment::Fill:
    //                    target_size = fs ? fs : cell_size;
    //                    break;
    //            }

    //            ds::dims<i32> pos = w->position(), size = w->size();
    //            pos[axis] = item_pos;
    //            size[axis] = target_size;
    //            w->set_position(pos);
    //            w->set_size(size);
    //            w->perform_layout(nvg_context);
    //        }
    //    }
    //}

    // void advanced_grid_layout::compute_layout(NVGcontext* nvg_context, const ui::widget*
    // ui::widget,
    //                                           std::vector<i32>* _grid) const
    //{
    //     ds::dims<i32> fs_w = ui::widget->fixed_size();
    //     ds::dims<i32> container_size(fs_w[0] ? fs_w[0] : ui::widget->width(),
    //                                  fs_w[1] ? fs_w[1] : ui::widget->height());

    //    ds::dims<i32> extra(2 * this->m_margin);
    //    const rl::Window* window = dynamic_cast<const rl::Window*>(ui::widget);
    //    if (window && !window->title().empty())
    //        extra[1] += ui::widget->theme()->m_window_header_height - this->m_margin / 2;

    //    container_size -= extra;

    //    for (i32 axis = 0; axis < 2; ++axis)
    //    {
    //        std::vector<i32>& grid = _grid[axis];
    //        const std::vector<i32>& sizes = axis == 0 ? m_cols : m_rows;
    //        const std::vector<float>& stretch = axis == 0 ? m_col_stretch : m_row_stretch;
    //        grid = sizes;

    //        for (i32 phase = 0; phase < 2; ++phase)
    //        {
    //            for (auto pair : m_anchor)
    //            {
    //                const ui::widget* w = pair.first;
    //                if (!w->visible() || dynamic_cast<const rl::Window*>(w) != nullptr)
    //                    continue;
    //                const Anchor& anchor = pair.second;
    //                if ((anchor.size[axis] == 1) != (phase == 0))
    //                    continue;
    //                i32 ps = w->preferred_size(nvg_context)[axis], fs = w->fixed_size()[axis];
    //                i32 target_size = fs ? fs : ps;

    //                if (anchor.pos[axis] + anchor.size[axis] > (i32)grid.size())
    //                    throw std::runtime_error(
    //                        "Advanced grid layout: ui::widget is out of bounds: " +
    //                        (std::string)anchor);

    //                i32 current_size = 0;
    //                float total_stretch = 0;
    //                for (i32 i = anchor.pos[axis]; i < anchor.pos[axis] + anchor.size[axis]; ++i)
    //                {
    //                    if (sizes[i] == 0 && anchor.size[axis] == 1)
    //                        grid[i] = std::max(grid[i], target_size);
    //                    current_size += grid[i];
    //                    total_stretch += stretch[i];
    //                }
    //                if (target_size <= current_size)
    //                    continue;
    //                if (total_stretch == 0)
    //                    throw std::runtime_error(
    //                        "Advanced grid layout: no space to place ui::widget: " +
    //                        (std::string)anchor);
    //                float amt = (target_size - current_size) / total_stretch;
    //                for (i32 i = anchor.pos[axis]; i < anchor.pos[axis] + anchor.size[axis]; ++i)
    //                    grid[i] += (i32)std::round(amt * stretch[i]);
    //            }
    //        }

    //        i32 current_size = std::accumulate(grid.begin(), grid.end(), 0);
    //        float total_stretch = std::accumulate(stretch.begin(), stretch.end(), 0.0f);
    //        if (current_size >= container_size[axis] || total_stretch == 0)
    //            continue;
    //        float amt = (container_size[axis] - current_size) / total_stretch;
    //        for (size_t i = 0; i < grid.size(); ++i)
    //            grid[i] += (i32)std::round(amt * stretch[i]);
    //    }
    //}
}
