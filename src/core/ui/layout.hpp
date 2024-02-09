#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    class Widget;

    enum class Alignment {
        Unknown = -1,  // Invalid / uninitialized alignment
        Minimum = 0,   // Take only as much space as is required.
        Center,        // Center align.
        Maximum,       // Take as much space as is allowed.
        Fill           // Fill according to preferred sizes.
    };

    enum class Orientation {
        Unknown = -1,    // Invalid / uninitialized orientation
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    enum Axis {
        Horizontal = 0,  // Layout expands on horizontal axis.
        Vertical         // Layout expands on vertical axis.
    };

    struct Anchor
    {
    public:
        Anchor() = default;

        Anchor(const u32 x, const u32 y, const Alignment horiz = Alignment::Fill,
               const Alignment vert = Alignment::Fill)
            : grid_pos{ static_cast<u8>(x), static_cast<u8>(y) }
            , cell_size{ 1, 1 }
            , align{ horiz, vert }
        {
        }

        Anchor(const u32 grid_x, const u32 grid_y, const u32 cell_span_width,
               const u32 cell_span_height, const Alignment horizontal_alignment = Alignment::Fill,
               const Alignment vertical_alignment = Alignment::Fill)
            : grid_pos{ grid_x, grid_y }
            , cell_size{ cell_span_width, cell_span_height }
            , align{ horizontal_alignment, vertical_alignment }
        {
        }

    public:
        ds::point<u32> grid_pos{ 0, 0 };
        ds::dims<u32> cell_size{ 0, 0 };
        std::array<Alignment, 2> align{};
    };

    class Layout : public ds::refcounted
    {
    public:
        template <typename T>
        struct Spacing
        {
            T horizontal{};
            T vertical{};
        };

    public:
        // Performs applies all Layout computations for the given widget.
        virtual void perform_layout(nvg::NVGcontext* nvc, Widget* w) const = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvc, const Widget* w) const = 0;

    protected:
        std::string name() const
        {
            return typeid(*this).name();
        }
    };

    // Simple layout that supports horizontal and vertical orientation.
    // Aside form defining the Layout interface for sizing and
    // performing the Layout, a BoxLayout only handles basic orientation,
    // margins and spacing.
    class BoxLayout final : public Layout
    {
    public:
        explicit BoxLayout(Orientation orientation,                  // horizontal or vertical
                           Alignment alignment = Alignment::Center,  // min, middle, max, full
                           f32 margin = 0.0f,                        // the widget margins
                           f32 spacing = 0.0f);                      // the widget spacing

        f32 margin() const;
        f32 spacing() const;
        Alignment alignment() const;
        Orientation orientation() const;

        void set_margin(f32 margin);
        void set_spacing(f32 spacing);
        void set_orientation(Orientation orientation);
        void set_alignment(Alignment alignment);

    public:
        virtual void perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const override;
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvg_context,
                                             const Widget* widget) const override;

    protected:
        f32 m_margin{ 0.0f };
        f32 m_spacing{ 0.0f };
        Orientation m_orientation{};
        Alignment m_alignment{};
    };

    // Special layout for widgets grouped by labels.
    // This widget resembles a box layout in that it arranges a set of widgets
    // vertically. All widgets are indented on the horizontal axis except for
    // Label widgets, which are not indented. This creates a pleasing layout where a number of
    // widgets are grouped under some high-level heading.
    class GroupLayout final : public Layout
    {
    public:
        explicit GroupLayout(const f32 margin = 15.0f, const f32 spacing = 6.0f,
                             const f32 group_spacing = 14.0f, const f32 group_indent = 20.0f)
            : m_margin{ margin }
            , m_spacing{ spacing }
            , m_group_spacing{ group_spacing }
            , m_group_indent{ group_indent }
        {
        }

        f32 margin() const;
        f32 spacing() const;
        f32 group_indent() const;
        f32 group_spacing() const;

        void set_margin(f32 margin);
        void set_spacing(f32 spacing);
        void set_group_indent(f32 group_indent);
        void set_group_spacing(f32 group_spacing);

    public:
        virtual void perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const override;
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvg_context,
                                             const Widget* widget) const override;

    protected:
        f32 m_margin{ 15.0f };
        f32 m_spacing{ 0.0f };
        f32 m_group_spacing{ 0.0f };
        f32 m_group_indent{ 0.0f };
    };

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
        virtual void perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const override;
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvg_context,
                                             const Widget* widget) const override;

    protected:
        void compute_layout(nvg::NVGcontext* nvg_context, const Widget* widget,
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

    class AdvancedGridLayout final : public Layout
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

    public:
        virtual void perform_layout(nvg::NVGcontext* nvg_context, Widget* widget) const override;
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvg_context,
                                             const Widget* widget) const override;

    protected:
        void compute_layout(nvg::NVGcontext* nvg_context, const Widget* widget,
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

namespace rl::ui {
    constexpr static auto format_as(const Alignment alignment)
    {
        switch (alignment)
        {
            case Alignment::Unknown:
                return "Unknown";
            case Alignment::Minimum:
                return "Minimum";
            case Alignment::Center:
                return "Center";
            case Alignment::Maximum:
                return "Maximum";
            case Alignment::Fill:
                return "Fill";
        }

        return "Unknown";
    }

    constexpr static auto format_as(const Orientation orientation)
    {
        switch (orientation)
        {
            case Orientation::Unknown:
                return "Unknown";
            case Orientation::Horizontal:
                return "Horizontal";
            case Orientation::Vertical:
                return "Vertical";
        }

        return "Unknown";
    }

    constexpr static auto format_as(const Axis axis)
    {
        switch (axis)
        {
            case Axis::Horizontal:
                return "Horizontal";
            case Axis::Vertical:
                return "Vertical";
        }

        return "Unknown";
    }

    static auto format_as(const Anchor& anchor)
    {
        return fmt::format("Format[pos=({}), size=({}), align=(h:{}, v:{})]", anchor.grid_pos,
                           anchor.cell_size, anchor.align[Axis::Horizontal],
                           anchor.align[Axis::Vertical]);
    }
}
