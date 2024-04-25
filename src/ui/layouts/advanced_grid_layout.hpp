#pragma once
#include <array>
#include <unordered_map>
#include <vector>

#include "ui/layouts/anchor.hpp"
#include "ui/layouts/layout.hpp"

namespace rl::ui {
    class Widget;

    class AdvancedGridLayout final : public OldLayout
    {
    public:
        explicit AdvancedGridLayout(const std::vector<f32>& cols = {},
                                    const std::vector<f32>& rows = {}, f32 margin = 0.0f);

        f32 margin() const;
        u32 col_count() const;
        u32 row_count() const;
        Anchor anchor(const Widget* widget) const;

        void set_margin(f32 margin);
        void append_row(f32 size, f32 stretch = 0.0f);
        void append_col(f32 size, f32 stretch = 0.0f);
        void set_row_stretch(i32 index, f32 stretch);
        void set_col_stretch(i32 index, f32 stretch);
        void set_anchor(const Widget* widget, const Anchor& anchor);

        virtual void apply_layout(nvg::Context* nvg_context, const Widget* widget) const override;
        virtual ds::dims<f32> computed_size(nvg::Context* nvg_context,
                                            const Widget* widget) const override;

        void compute_layout(nvg::Context* nvg_context, const Widget* widget,
                            std::array<std::vector<f32>, 2>& grid_cell_sizes) const;

    protected:
        // The columns of this AdvancedGridLayout.
        std::vector<f32> m_cols{};
        // The rows of this AdvancedGridLayout.
        std::vector<f32> m_rows{};
        // The stretch for each column of this AdvancedGridLayout.
        std::vector<f32> m_col_stretch{};
        // The stretch for each row of this AdvancedGridLayout.
        std::vector<f32> m_row_stretch{};
        // The mapping of widgets to their specified anchor points.
        std::unordered_map<const Widget*, Anchor> m_anchor{};
        // The margin around this AdvancedGridLayout.
        f32 m_margin{ 0.0f };

    private:
        enum LayoutComputePhase {
            ComputeCellSize = 0,
            MulitCellMerge = 1,
        };
    };
}