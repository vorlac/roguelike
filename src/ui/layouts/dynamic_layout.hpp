#pragma once

#include <array>
#include <concepts>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

#include "layout.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;
    class Canvas;

    // clang-format off
    //struct CellProperties
    //{
    //    Alignment alignment{ Alignment::Fill };
    //};

    enum class ColumnProperty {
        MinWidth       = 1 << 0,  // The min column width
        MaxWidth       = 1 << 1,  // The max column width
        FixedWidth     = 1 << 2,  // Fixed column width that won't be adjusted during resizing
        DynamicScaling = 1 << 3,  // If/how the column is scaled when the layout is resized
        Interactive    = 1 << 4,  // Enables/disables mouse/kb interaction for the edges or cells
        Outline        = 1 << 3,  // Defines if/how a column outline is drawn
        Spacing        = 1 << 4,  // 
    };

    // clang-format on

    // class DynamicLayout : public Layout
    //{
    // public:
    //     enum class Properties {
    //         Column
    //     };

    // public:
    //     explicit DynamicLayout() = default;

    //    // init emptry columns x rows sized layout table
    //    explicit DynamicLayout(const u32 columns, const u32 rows)
    //    {
    //        m_grid.reserve(columns);
    //        for (auto&& [col_idx, row] : std::ranges::views::enumerate(m_grid))
    //        {
    //            row.resize(rows, nullptr);
    //            m_grid[col_idx] = std::move(row);
    //        }
    //    }

    //    template <typename TParent>
    //        requires std::derived_from<std::remove_pointer_t<TParent>, Widget>
    //    explicit DynamicLayout(TParent parent, const std::vector<std::vector<Widget*>>&
    //    layout_table)
    //    {
    //        this->set_widget_table(parent, layout_table);
    //    }

    // public:
    //     template <typename TParent>
    //         requires std::derived_from<std::remove_pointer_t<TParent>, Widget>
    //     bool set_widget_table(TParent parent, const std::vector<std::vector<Widget*>>&
    //     layout_table)
    //     {
    //         m_grid.clear();
    //         for (const auto& row : layout_table)
    //         {
    //             ++m_row_count;
    //             u32 col_count{ 0 };
    //             for (const auto& w : row)
    //             {
    //                 if (col_count)
    //                     if (w != nullptr)
    //                         parent->add_child(w);
    //             }
    //             m_grid.push_back(row);
    //         }

    //        return !m_grid.empty();
    //    }

    //    // Performs applies all Layout computations for the given widget.
    //    virtual void apply_layout(nvg::Context* nvc, const Widget* w) const override
    //    {
    //    }

    //    // Compute the preferred size for a given Layout and widget
    //    virtual ds::dims<f32> computed_size(nvg::Context* nvc, const Widget* w) const override
    //    {
    //        return {};
    //    }

    // private:
    //     std::vector<std::vector<Widget*>> m_grid{};
    //     u32 m_col_count{ 0 };
    //     u32 m_row_count{ 0 };
    // };
}
