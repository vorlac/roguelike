#pragma once

#include <array>
#include <vector>

#include "ui/layouts/layout.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;

    class GridLayout final : public OldLayout {
    public:
        explicit GridLayout(const Alignment orientation = Alignment::Horizontal,
                            const f32 resolution = 2.0f,
                            const Placement_OldAlignment alignment = Placement_OldAlignment::Center,
                            const f32 margin = 0.0f, const f32 spacing = 0.0f)
            : m_margin{ margin }
            , m_resolution{ resolution }
            , m_spacing{ spacing, spacing }
            , m_orientation{ orientation }
            , m_default_alignment{ alignment, alignment } {
        }

        f32 margin() const;
        f32 resolution() const;
        f32 spacing(Axis axis) const;
        Alignment orientation() const;
        Placement_OldAlignment alignment(Axis axis, i32 item) const;

        void set_margin(f32 margin);
        void set_resolution(f32 resolution);
        void set_spacing(f32 spacing);
        void set_spacing(Axis axis, f32 spacing);
        void set_orientation(Alignment orientation);
        void set_col_alignment(Placement_OldAlignment value);
        void set_row_alignment(Placement_OldAlignment value);
        void set_col_alignment(const std::vector<Placement_OldAlignment>& value);
        void set_row_alignment(const std::vector<Placement_OldAlignment>& value);

    public:
        virtual void apply_layout(nvg::Context* nvg_context, const Widget* widget) const override;
        virtual ds::dims<f32> computed_size(nvg::Context* nvg_context,
                                            const Widget* widget) const override;

    protected:
        void compute_layout(nvg::Context* nvg_context, const Widget* widget,
                            std::array<std::vector<f32>, 2>& grid) const;

    protected:
        // The margin around this GridLayout.
        f32 m_margin{ 0.0f };
        // The number of rows or columns before starting a new one, depending on the Orientation.
        f32 m_resolution{ 0.0f };
        // The spacing used for each dimension.
        ds::vector2<f32> m_spacing{ 0.0f, 0.0f };
        //  The Orientation of the GridLayout.
        Alignment m_orientation{ Alignment::None };
        // The default Alignment of the GridLayout.
        std::array<Placement_OldAlignment, 2> m_default_alignment{ { {}, {} } };
        // The actual Alignment being used for each column/row
        std::array<std::vector<Placement_OldAlignment>, 2> m_alignment{ { {}, {} } };
    };

}
