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

    enum Alignment {
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

        Anchor(u32 x, u32 y, Alignment horiz = Alignment::Fill, Alignment vert = Alignment::Fill)
            : grid_pos{ static_cast<u8>(x), static_cast<u8>(y) }
            , cell_size{ 1, 1 }
            , align{ horiz, vert }
        {
        }

        Anchor(u32 grid_x, u32 grid_y, u32 cell_span_width, u32 cell_span_height,
               Alignment horizontal_alignment = Alignment::Fill,
               Alignment vertical_alignment = Alignment::Fill)
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

    /// @brief
    ///     The common Layout interface
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
        /// @brief
        ///     Performs applies all Layout computations for the given widget.
        ///
        /// @param  nvc
        ///     The NanoVG context being used for drawing.
        /// @param  w
        ///     The Widget whose child widgets will be positioned by the Layout class.
        virtual void perform_layout(nvg::NVGcontext* nvc, Widget* w) const = 0;

        /// @brief
        ///     Compute the preferred size for a given Layout and widget
        ///
        /// @param  nvc
        ///     The NanoVG context being used for drawing.
        /// @param  w
        ///     Widget, whose preferred size should be computed.
        ///
        /// @returns
        ///     A ds::dims{i32}
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvc, const Widget* w) const = 0;
    };

    /// @brief
    ///     Box Layout is a simple Layout that supports horizontal and vertical
    ///     orientation.
    ///
    ///     Aside form defining the Layout interface for sizing and
    ///     performing the Layout, a box_layout only handles basic orientation,
    ///     margins and spacing.
    class BoxLayout : public Layout
    {
    public:
        BoxLayout(Orientation orientation,                  // horizontal or vertical
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
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvc, const Widget* w) const override;
        virtual void perform_layout(nvg::NVGcontext* nvc, Widget* w) const override;

    protected:
        f32 m_margin{ 0.0f };
        f32 m_spacing{ 0.0f };
        Orientation m_orientation{};
        Alignment m_alignment{};
    };

    /// Group Layout
    ///
    /// @brief
    ///     Aside form definint the Layout interface for sizing and performing the Layout, a
    ///     box_layout only handles basic orientation, margins and spacing.
    class GroupLayout : public Layout
    {
    public:
        GroupLayout(f32 margin = 15.0f, f32 spacing = 6.0f, f32 group_spacing = 14.0f,
                    f32 group_indent = 20.0f)
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
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvc, const Widget* w) const override;
        virtual void perform_layout(nvg::NVGcontext* nvc, Widget* w) const override;

    protected:
        f32 m_margin{ 15.0f };
        f32 m_spacing{ 0.0f };
        f32 m_group_spacing{ 0.0f };
        f32 m_group_indent{ 0.0f };
    };

    class GridLayout : public Layout
    {
    public:
        GridLayout(Orientation orientation = Orientation::Horizontal,  //
                   f32 resolution = 2.0f,                              //
                   Alignment alignment = Alignment::Center,            //
                   f32 margin = 0.0f,                                  //
                   f32 spacing = 0.0f)                                 //
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
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvc, const Widget* w) const override;
        virtual void perform_layout(nvg::NVGcontext* nvc, Widget* w) const override;

    protected:
        void compute_layout(nvg::NVGcontext* nvg_context, const Widget* widget,
                            std::array<std::vector<f32>, 2>& grid) const;

    protected:
        f32 m_margin{ 0.0f };
        f32 m_resolution{ 0.0f };
        ds::vector2<f32> m_spacing{ 0.0f, 0.0f };
        Orientation m_orientation{ Orientation::Unknown };
        std::array<Alignment, 2> m_default_alignment{ { {}, {} } };
        std::array<std::vector<Alignment>, 2> m_alignment{ { {}, {} } };
    };

    class AdvancedGridLayout : public Layout
    {
    public:
        AdvancedGridLayout(const std::vector<f32>& cols = {}, const std::vector<f32>& rows = {},
                           f32 margin = 0.0f);

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
        virtual ds::dims<f32> preferred_size(nvg::NVGcontext* nvc, const Widget* w) const override;
        virtual void perform_layout(nvg::NVGcontext* nvc, Widget* w) const override;

    protected:
        void compute_layout(nvg::NVGcontext* nvg_context, const Widget* widget,
                            std::array<std::vector<f32>, 2>& grid) const;

    protected:
        std::vector<f32> m_cols{};
        std::vector<f32> m_rows{};
        std::vector<f32> m_col_stretch{};
        std::vector<f32> m_row_stretch{};
        std::unordered_map<const Widget*, Anchor> m_anchor{};
        f32 m_margin{ 0.0f };

    private:
        enum LayoutComputePhase {
            ComputeCellSize = 0,
            MulitCellMerge = 1,
        };
    };
}

namespace rl::ui {
    constexpr static auto format_as(Alignment alignment)
    {
        switch (alignment)
        {
            default:
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
    }

    constexpr static auto format_as(Orientation orientation)
    {
        switch (orientation)
        {
            default:
            case Orientation::Unknown:
                return "Unknown";
            case Orientation::Horizontal:
                return "Horizontal";
            case Orientation::Vertical:
                return "Vertical";
        }
    }

    constexpr static auto format_as(Axis axis)
    {
        switch (axis)
        {
            default:
                return "Unknown";
            case Axis::Horizontal:
                return "Horizontal";
            case Axis::Vertical:
                return "Vertical";
        }
    }

    static auto format_as(const Anchor& anchor)
    {
        return fmt::format("Format[pos=({}), size=({}), align=(h:{}, v:{})]", anchor.grid_pos,
                           anchor.cell_size, anchor.align[Axis::Horizontal],
                           anchor.align[Axis::Vertical]);
    }
}
