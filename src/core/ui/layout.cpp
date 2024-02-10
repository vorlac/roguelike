#include <numeric>
#include <utility>

#include "core/ui/dialog.hpp"
#include "core/ui/label.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    BoxLayout::BoxLayout(const Orientation orientation, const Alignment alignment, const f32 margin,
                         const f32 spacing)
        : m_margin{ margin }
        , m_spacing{ spacing }
        , m_orientation{ orientation }
        , m_alignment{ alignment }
    {
        scoped_log();
    }

    ds::dims<f32> BoxLayout::preferred_size(nvg::NVGcontext* nvg_context, const Widget* widget) const
    {
        scoped_log();

        ds::dims size{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        f32 y_offset{ 0.0f };
        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
        {
            const auto header_size{ dialog->header_height() };
            if (m_orientation == Orientation::Vertical)
                size.height += header_size - (m_margin / 2.0f);
            else
                y_offset = header_size;
        }

        bool first_child{ true };
        for (const auto child : widget->children())
        {
            if (!child->visible())
                continue;

            if (!first_child)
                size.height += m_spacing;

            const ds::dims ps{ child->preferred_size() };
            const ds::dims fs{ child->fixed_size() };
            const ds::dims target_size{
                std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() != 0.0f ? fs.height
                                                                                   : ps.height,
            };

            first_child = false;
            size.width += target_size.width;
            size.height = std::max(size.height, target_size.height + (2.0f * m_margin));
        }

        return size + ds::dims{ 0.0f, y_offset };
    }

    void BoxLayout::perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const
    {
        scoped_log();

        const ds::dims fs_w{ widget->fixed_size() };
        ds::dims container_size{
            std::fabs(fs_w.width) > std::numeric_limits<f32>::epsilon() ? fs_w.width
                                                                        : widget->width(),
            std::fabs(fs_w.height) > std::numeric_limits<f32>::epsilon() ? fs_w.height
                                                                         : widget->height(),
        };

        f32 position{ m_margin };
        f32 y_offset{ 0.0f };

        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
        {
            if (m_orientation == Orientation::Vertical)
                position += dialog->header_height() - (m_margin / 2.0f);
            else
            {
                y_offset = dialog->header_height();
                container_size.height -= y_offset;
            }
        }

        bool first_child{ true };
        for (auto& child : widget->children())
        {
            if (!child->visible())
                continue;

            if (!first_child)
                position += m_spacing;

            first_child = false;
            const ds::dims ps{ child->preferred_size() };
            const ds::dims fs{ child->fixed_size() };
            ds::dims target_size{
                std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height : ps.height,
            };

            ds::point pos{
                0.0f,
                y_offset,
            };

            pos.x = position;
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
                    target_size.height = std::fabs(fs.height) > std::numeric_limits<f32>::epsilon()
                                           ? fs.height
                                           : (container_size.height - m_margin * 2.0f);
                    break;
                case Alignment::Unknown:
                    assert_cond(false);
                    break;
            }

            child->set_position(pos);
            child->set_size(target_size);
            child->perform_layout();
            position += target_size.width;
        }
    }

    Orientation BoxLayout::orientation() const
    {
        scoped_log();
        return m_orientation;
    }

    void BoxLayout::set_orientation(const Orientation orientation)
    {
        scoped_log();
        m_orientation = orientation;
    }

    Alignment BoxLayout::alignment() const
    {
        scoped_log();
        return m_alignment;
    }

    void BoxLayout::set_alignment(const Alignment alignment)
    {
        scoped_log();
        m_alignment = alignment;
    }

    f32 BoxLayout::margin() const
    {
        return m_margin;
    }

    void BoxLayout::set_margin(const f32 margin)
    {
        scoped_log();
        m_margin = margin;
    }

    f32 BoxLayout::spacing() const
    {
        scoped_log();
        return m_spacing;
    }

    void BoxLayout::set_spacing(const f32 spacing)
    {
        scoped_log();
        m_spacing = spacing;
    }

    //======================================================================

    ds::dims<f32> GroupLayout::preferred_size(nvg::NVGcontext* nvg_context,
                                              const Widget* widget) const
    {
        scoped_log();

        f32 height{ m_margin };
        f32 width{ 2.0f * m_margin };

        auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            height += dialog->header_height() - (m_margin / 2.0f);

        bool first{ true };
        bool indent{ false };
        for (const auto c : widget->children())
        {
            if (!c->visible())
                continue;

            const auto label{ dynamic_cast<const Label*>(c) };
            if (!first)
                height += (label == nullptr) ? m_spacing : m_group_spacing;

            first = false;
            ds::dims ps{ c->preferred_size() };
            ds::dims fs{ c->fixed_size() };
            const ds::dims target_size{
                std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height : ps.height,
            };

            const bool indent_cur{ indent && label == nullptr };
            height += target_size.height;
            width = std::max(width, target_size.width + (2.0f * m_margin) +
                                        (indent_cur ? m_group_indent : 0.0f));
            if (label != nullptr)
                indent = !label->text().empty();
        }

        height += m_margin;
        return ds::dims{ width, height };
    }

    void GroupLayout::perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const
    {
        scoped_log();

        f32 height{ m_margin };
        const f32 available_width{ (std::fabs(widget->fixed_width()) >
                                            std::numeric_limits<f32>::epsilon()
                                        ? widget->fixed_width()
                                        : widget->width()) -
                                   (2.0f * m_margin) };

        auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            height += dialog->header_height() - (m_margin / 2.0f);

        bool indent{ false };
        bool first_child{ true };
        for (const auto child : widget->children())
        {
            if (!child->visible())
                continue;

            auto label{ dynamic_cast<const Label*>(child) };
            if (!first_child)
                height += (label == nullptr) ? m_spacing : m_group_spacing;

            first_child = false;
            const bool indent_cur{ indent && label == nullptr };

            const ds::dims fs{ child->fixed_size() };
            const ds::dims ps{
                available_width - (indent_cur ? m_group_indent : 0.0f),
                child->preferred_size().height,
            };

            const ds::dims target_size{
                std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height : ps.height,
            };

            child->set_position({
                m_margin + (indent_cur ? m_group_indent : 0.0f),
                height,
            });

            child->set_size(target_size);
            child->perform_layout();

            height += target_size.height;
            if (label != nullptr)
                indent = !label->text().empty();
        }
    }

    f32 GroupLayout::margin() const
    {
        scoped_log();
        return m_margin;
    }

    void GroupLayout::set_margin(const f32 margin)
    {
        scoped_log();
        m_margin = margin;
    }

    f32 GroupLayout::spacing() const
    {
        scoped_log();
        return m_spacing;
    }

    void GroupLayout::set_spacing(const f32 spacing)
    {
        scoped_log();
        m_spacing = spacing;
    }

    f32 GroupLayout::group_indent() const
    {
        scoped_log();
        return m_group_indent;
    }

    void GroupLayout::set_group_indent(const f32 group_indent)
    {
        scoped_log();
        m_group_indent = group_indent;
    }

    f32 GroupLayout::group_spacing() const
    {
        scoped_log();
        return m_group_spacing;
    }

    void GroupLayout::set_group_spacing(const f32 group_spacing)
    {
        scoped_log();
        m_group_spacing = group_spacing;
    }

    //==================================================================

    ds::dims<f32> GridLayout::preferred_size(nvg::NVGcontext* nvg_context,
                                             const Widget* widget) const
    {
        scoped_log();

        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        ds::dims pref_size{
            (2.0f * m_margin) + std::accumulate(grid[0].begin(), grid[0].end(), 0.0f) +
                (std::max(static_cast<f32>(grid[0].size()) - 1.0f, 0.0f) * m_spacing.x),
            (2.0f * m_margin) + std::accumulate(grid[1].begin(), grid[1].end(), 0.0f) +
                std::max(static_cast<f32>(grid[1].size()) - 1.0f, 0.0f) * m_spacing.y,
        };

        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            pref_size.height += dialog->header_height() - (m_margin / 2.0f);

        return pref_size;
    }

    void GridLayout::compute_layout(nvg::NVGcontext* nvg_context, const Widget* widget,
                                    std::array<std::vector<f32>, 2>& grid) const
    {
        scoped_log();

        const i32 axis1{ std::to_underlying(m_orientation) };
        const i32 axis2{ (axis1 + 1) % 2 };
        const i32 num_children{ widget->child_count() };

        i32 visible_children{ 0 };
        for (auto&& w : widget->children())
            visible_children += w->visible() ? 1 : 0;

        const ds::dims dim{
            m_resolution,
            (static_cast<f32>(visible_children) + m_resolution - 1.0f) / m_resolution,
        };

        grid[axis1].clear();
        grid[axis1].resize(static_cast<size_t>(dim.width), 0.0f);
        grid[axis2].clear();
        grid[axis2].resize(static_cast<size_t>(dim.height), 0.0f);

        const auto dim_axis2{ static_cast<i32>(axis2 == Axis::Horizontal ? dim.width : dim.height) };

        i32 child{ 0 };
        for (i32 i2 = 0; i2 < dim_axis2; i2++)
        {
            const i32 dim_axis1{ static_cast<i32>(axis1 == Axis::Horizontal ? dim.width
                                                                            : dim.height) };

            for (i32 i1 = 0; i1 < dim_axis1; i1++)
            {
                const Widget* w{ nullptr };

                do
                {
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

                auto& target_size_axis1{ axis1 == std::to_underlying(Axis::Horizontal)
                                             ? target_size.width
                                             : target_size.height };
                auto& target_size_axis2{ axis2 == std::to_underlying(Axis::Horizontal)
                                             ? target_size.width
                                             : target_size.height };

                grid[axis1][i1] = std::max(grid[axis1][i1], target_size_axis1);
                grid[axis2][i2] = std::max(grid[axis2][i2], target_size_axis2);
            }
        }
    }

    void GridLayout::perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const
    {
        scoped_log();

        const ds::dims fs_w{ widget->fixed_size() };
        const ds::dims container_size{
            std::fabs(fs_w.width) > std::numeric_limits<f32>::epsilon() ? fs_w.width
                                                                        : widget->width(),
            std::fabs(fs_w.height) > std::numeric_limits<f32>::epsilon() ? fs_w.height
                                                                         : widget->height(),
        };
        // Compute minimum row / column sizes
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        const std::array dim = {
            static_cast<f32>(grid[Axis::Horizontal].size()),
            static_cast<f32>(grid[Axis::Vertical].size()),
        };

        ds::dims extra{ 0.0f, 0.0f };
        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            extra.height += dialog->header_height() - (m_margin / 2.0f);

        // Strech to size provided by widget
        for (const auto cur_axis : { Axis::Horizontal, Axis::Vertical })
        {
            f32 grid_size{ (2.0f * m_margin) +
                           (cur_axis == Axis::Horizontal ? extra.width : extra.height) };

            const i32 axis_idx{ std::to_underlying(cur_axis) };
            for (const auto& s : grid[axis_idx])
            {
                grid_size += s;
                if (1.0f + axis_idx < dim[axis_idx])
                    grid_size += this->spacing(cur_axis);
            }

            const f32& axis_size{ cur_axis == Axis::Horizontal ? container_size.width
                                                               : container_size.height };
            if (grid_size < axis_size)
            {
                // Re-distribute remaining space evenly
                const f32 gap{ axis_size - grid_size };
                const f32 g{ gap / dim[axis_idx] };

                f32 rest{ gap - g * dim[axis_idx] };
                for (i32 i = 0; i < dim[axis_idx]; ++i)
                    grid[axis_idx][i] += g;
                for (i32 i = 0; rest > 0 && i < dim[axis_idx]; --rest, ++i)
                    grid[axis_idx][i] += 1.0f;
            }
        }

        const i32 num_children{ widget->child_count() };
        const i32 axis1{ std::to_underlying(m_orientation) };
        const i32 axis2{ (axis1 + 1) % 2 };

        const ds::dims<i32> start_offset{ extra + m_margin };
        const ds::point start{ start_offset.width, start_offset.height };
        ds::point pos{ start };

        i32& axis1_pos{ axis1 == std::to_underlying(Orientation::Horizontal) ? pos.x : pos.y };
        i32& axis2_pos{ axis2 == std::to_underlying(Orientation::Horizontal) ? pos.x : pos.y };

        i32 child_idx{ 0 };
        for (i32 i2 = 0; i2 < static_cast<i32>(dim[axis2]); i2++)
        {
            const i32& s{ m_orientation == Orientation::Horizontal ? start.x : start.y };
            i32& p{ m_orientation == Orientation::Horizontal ? pos.x : pos.y };

            p = s;
            for (i32 i1 = 0; i1 < static_cast<i32>(dim[axis1]); i1++)
            {
                Widget* child{ nullptr };
                do
                {
                    if (child_idx >= num_children)
                        return;

                    child = widget->children()[child_idx++];
                }
                while (!child->visible());

                const ds::dims ps{ child->preferred_size() };
                ds::dims fs{ child->fixed_size() };
                ds::dims target_size{
                    std::fabs(fs.width) > std::numeric_limits<f32>::epsilon() ? fs.width : ps.width,
                    std::fabs(fs.height) > std::numeric_limits<f32>::epsilon() ? fs.height
                                                                               : ps.height,
                };

                ds::point item_pos{ pos };
                for (i32 j = 0; j < 2; j++)
                {
                    const i32 axis_idx{ (axis1 + j) % 2 };
                    const i32 item_idx{ j == 0 ? i1 : i2 };
                    const Alignment align{ this->alignment(static_cast<Axis>(axis_idx), item_idx) };

                    i32& item_axis_pos{ axis_idx == Axis::Horizontal ? item_pos.x : item_pos.y };
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
                        case Alignment::Unknown:
                            assert_cond(false);
                            break;
                    }
                }

                child->set_position(item_pos);
                child->set_size(std::move(target_size));
                child->perform_layout();

                axis1_pos += static_cast<i32>(
                    grid[axis1][i1] + this->spacing(static_cast<Axis>(axis1)));
            }

            axis2_pos += static_cast<i32>(grid[axis2][i2] + this->spacing(static_cast<Axis>(axis2)));
        }
    }

    Orientation GridLayout::orientation() const
    {
        scoped_log();
        return m_orientation;
    }

    void GridLayout::set_orientation(const Orientation orientation)
    {
        scoped_log();
        m_orientation = orientation;
    }

    f32 GridLayout::resolution() const
    {
        scoped_log();
        return m_resolution;
    }

    void GridLayout::set_resolution(const f32 resolution)
    {
        scoped_log();
        m_resolution = resolution;
    }

    f32 GridLayout::spacing(const Axis axis) const
    {
        scoped_log();

        switch (axis)
        {
            case Axis::Horizontal:
                return m_spacing.x;
            case Axis::Vertical:
                return m_spacing.y;
            default:
                runtime_assert(false, "invalid axis value");
                return -1;
        }
    }

    void GridLayout::set_spacing(const Axis axis, const f32 spacing)
    {
        scoped_log();

        switch (axis)
        {
            case Axis::Horizontal:
                m_spacing.x = spacing;
                break;
            case Axis::Vertical:
                m_spacing.y = spacing;
                break;
            default:
                runtime_assert(false, "invalid axis value");
                break;
        }
    }

    void GridLayout::set_spacing(const f32 spacing)
    {
        scoped_log();
        m_spacing = ds::vector2{ spacing, spacing };
    }

    f32 GridLayout::margin() const
    {
        scoped_log();
        return m_margin;
    }

    void GridLayout::set_margin(const f32 margin)
    {
        scoped_log();
        m_margin = margin;
    }

    Alignment GridLayout::alignment(const Axis axis, const i32 item) const
    {
        scoped_log();
        if (item < static_cast<i32>(m_alignment[axis].size()))
            return m_alignment[axis][item];
        else
            return m_default_alignment[axis];
    }

    void GridLayout::set_col_alignment(const Alignment value)
    {
        scoped_log();
        m_default_alignment[Axis::Horizontal] = value;
    }

    void GridLayout::set_row_alignment(const Alignment value)
    {
        scoped_log();
        m_default_alignment[Axis::Vertical] = value;
    }

    void GridLayout::set_col_alignment(const std::vector<Alignment>& value)
    {
        scoped_log();
        m_alignment[Axis::Horizontal] = value;
    }

    void GridLayout::set_row_alignment(const std::vector<Alignment>& value)
    {
        scoped_log();
        m_alignment[Axis::Vertical] = value;
    }

    //=======================================================================

    AdvancedGridLayout::AdvancedGridLayout(const std::vector<f32>& cols,
                                           const std::vector<f32>& rows, const f32 margin)
        : m_cols(cols)
        , m_rows(rows)
        , m_margin(margin)
    {
        scoped_log();

        m_col_stretch.resize(m_cols.size(), 0.0f);
        m_row_stretch.resize(m_rows.size(), 0.0f);
    }

    ds::dims<f32> AdvancedGridLayout::preferred_size(nvg::NVGcontext* nvg_context,
                                                     const Widget* widget) const
    {
        scoped_log();

        // Compute minimum row / column sizes
        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        const ds::dims size{
            std::accumulate(grid[Axis::Horizontal].begin(), grid[Axis::Horizontal].end(), 0.0f),
            std::accumulate(grid[Axis::Vertical].begin(), grid[Axis::Vertical].end(), 0.0f),
        };

        ds::dims extra{
            2.0f * m_margin,
            2.0f * m_margin,
        };

        auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            extra.height += widget->theme()->dialog_header_height - m_margin / 2.0f;

        return size + extra;
    }

    void AdvancedGridLayout::perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const
    {
        scoped_log();

        std::array<std::vector<f32>, 2> grid{ { {}, {} } };
        this->compute_layout(nvg_context, widget, grid);

        grid[Axis::Horizontal].insert(grid[Axis::Horizontal].begin(), m_margin);

        auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog == nullptr || dialog->title().empty())
            grid[Axis::Vertical].insert(grid[Axis::Vertical].begin(), m_margin);
        else if (dialog != nullptr)
            grid[Axis::Vertical].insert(grid[Axis::Vertical].begin(),
                                        dialog->header_height() + m_margin / 2.0f);
        else
        {
            grid[Axis::Vertical].insert(grid[Axis::Vertical].begin(),
                                        widget->theme()->dialog_header_height + m_margin / 2.0f);
        }

        for (const Axis axis : { Axis::Horizontal, Axis::Vertical })
        {
            std::vector<f32>& axis_grids{ grid[axis] };
            for (size_t i = 1; i < axis_grids.size(); ++i)
                axis_grids[i] += axis_grids[i - 1];

            for (Widget* child : widget->children())
            {
                const Dialog* child_dialog{ dynamic_cast<Dialog*>(child) };
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
                switch (anchor.align[axis])
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
                        target_size = std::fabs(fs) > std::numeric_limits<f32>::epsilon()
                                        ? fs
                                        : cell_size;
                        break;
                    case Alignment::Unknown:
                        assert_cond(false);
                        break;
                }

                ds::point pos{ child->position() };
                ds::dims size{ child->size() };
                f32& item_axis_pos{ axis == Axis::Horizontal ? pos.x : pos.y };
                f32& item_axis_size{ axis == Axis::Horizontal ? size.width : size.height };
                item_axis_pos = item_pos;
                item_axis_size = target_size;

                child->set_position(pos);
                child->set_size(size);
                child->perform_layout();
            }
        }
    }

    void AdvancedGridLayout::compute_layout(nvg::NVGcontext* nvg_context, const Widget* widget,
                                            std::array<std::vector<f32>, 2>& grid_cell_sizes) const
    {
        scoped_log();

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

        const auto dialog{ dynamic_cast<const Dialog*>(widget) };
        if (dialog != nullptr && !dialog->title().empty())
            extra.height += dialog->header_height() - (m_margin / 2.0f);

        container_size -= extra;
        for (const Axis axis : { Axis::Horizontal, Axis::Vertical })
        {
            std::vector<f32>& grid{ grid_cell_sizes[axis] };
            const bool col_axis{ axis == Axis::Horizontal };
            const std::vector<f32>& sizes{ col_axis ? m_cols : m_rows };
            const std::vector<f32>& stretch{ col_axis ? m_col_stretch : m_row_stretch };

            grid = sizes;
            for (const auto phase : { ComputeCellSize, MulitCellMerge })
            {
                for (const auto& [layout_widget, widget_anchor] : m_anchor)
                {
                    const Widget* w{ layout_widget };
                    const auto anchor_window{ dynamic_cast<const Dialog*>(w) };
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
                                   "Advanced grid layout: widget is out of bounds: {}", anchor);

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

                    runtime_assert(total_stretch != 0,
                                   "Advanced grid layout: no space to place widget: {}", anchor);

                    const f32 amt{ (target_size - current_size) / total_stretch };
                    for (u32 i = axis_anchor_pos; i < axis_anchor_pos + axis_anchor_size; ++i)
                        grid[i] += std::round(amt * stretch[i]);
                }
            }

            const f32 current_size{ std::accumulate(grid.begin(), grid.end(), 0.0f) };
            const f32 total_stretch{ std::accumulate(stretch.begin(), stretch.end(), 0.0f) };
            const f32 axis_container_size{ axis == Axis::Horizontal ? container_size.width
                                                                    : container_size.height };

            if (current_size >= axis_container_size || total_stretch == 0.0f)
                continue;

            const f32 amt{ (axis_container_size - current_size) / total_stretch };
            for (size_t i = 0; i < grid.size(); ++i)
                grid[i] += std::round(amt * stretch[i]);
        }
    }

    f32 AdvancedGridLayout::margin() const
    {
        scoped_log();
        return m_margin;
    }

    void AdvancedGridLayout::set_margin(const f32 margin)
    {
        scoped_log();
        m_margin = margin;
    }

    u32 AdvancedGridLayout::col_count() const
    {
        scoped_log();
        return static_cast<u32>(m_cols.size());
    }

    u32 AdvancedGridLayout::row_count() const
    {
        scoped_log();
        return static_cast<u32>(m_rows.size());
    }

    void AdvancedGridLayout::append_row(const f32 size, const f32 stretch)
    {
        scoped_log();
        m_rows.push_back(size);
        m_row_stretch.push_back(stretch);
    }

    void AdvancedGridLayout::append_col(const f32 size, const f32 stretch)
    {
        scoped_log();
        m_cols.push_back(size);
        m_col_stretch.push_back(stretch);
    }

    void AdvancedGridLayout::set_row_stretch(const i32 index, const f32 stretch)
    {
        scoped_log();
        m_row_stretch.at(index) = stretch;
    }

    void AdvancedGridLayout::set_col_stretch(const i32 index, const f32 stretch)
    {
        scoped_log();
        m_col_stretch.at(index) = stretch;
    }

    void AdvancedGridLayout::set_anchor(const Widget* widget, const Anchor& anchor)
    {
        scoped_log();
        m_anchor[widget] = anchor;
    }

    Anchor AdvancedGridLayout::anchor(const Widget* widget) const
    {
        scoped_log();

        const auto it{ m_anchor.find(widget) };
        runtime_assert(it != m_anchor.end(), "Widget was not registered with the grid layout!");
        return it->second;
    }
}
