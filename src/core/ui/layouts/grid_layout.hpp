#pragma once

#include <vector>

#include "core/ui/layouts/layout.hpp"
#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    class Widget;

    class GridLayout final : public Layout
    {
    public:
        explicit GridLayout(const Orientation orientation = Orientation::Horizontal,  //
                            const f32 resolution = 2.0f,                              //
                            const Alignment alignment = Alignment::Center,            //
                            const f32 margin = 0.0f,                                  //
                            const f32 spacing = 0.0f)                                 //
            : m_margin{ margin }
            , m_resolution{ resolution }
            , m_spacing{ spacing, spacing }
            , m_orientation{ orientation }
            , m_default_alignment{ alignment, alignment }
        {
        }

        f32 margin() const;
        f32 resolution() const;
        f32 spacing(Axis axis) const;
        Orientation orientation() const;
        Alignment alignment(Axis axis, i32 item) const;

        void set_margin(f32 margin);
        void set_resolution(f32 resolution);
        void set_spacing(f32 spacing);
        void set_spacing(Axis axis, f32 spacing);
        void set_orientation(Orientation orientation);
        void set_col_alignment(Alignment value);
        void set_row_alignment(Alignment value);
        void set_col_alignment(const std::vector<Alignment>& value);
        void set_row_alignment(const std::vector<Alignment>& value);

    public:
        virtual void perform_layout(nvg::Context* nvg_context, Widget* widget) const override;
        virtual ds::dims<f32> preferred_size(nvg::Context* nvg_context,
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
        Orientation m_orientation{ Orientation::Unknown };
        // The default Alignment of the GridLayout.
        std::array<Alignment, 2> m_default_alignment{ { {}, {} } };
        // The actual Alignment being used for each column/row
        std::array<std::vector<Alignment>, 2> m_alignment{ { {}, {} } };
    };

}
