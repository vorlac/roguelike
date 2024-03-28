#pragma once

#include "anchor.hpp"
#include "ds/color.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/conversions.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;

    struct Spacing
    {
        f32 horizontal{ 0.0f };
        f32 vertical{ 0.0f };
    };

    struct Margin
    {
        f32 top{ 0.0f };
        f32 bottom{ 0.0f };
        f32 left{ 0.0f };
        f32 right{ 0.0f };
    };

    struct Padding
    {
        f32 top{ 0.0f };
        f32 bottom{ 0.0f };
        f32 left{ 0.0f };
        f32 right{ 0.0f };
    };

    struct CellProperties
    {
        Alignment alignment{ Alignment::Fill };

        // the gap of space measured from the inner text/icon/etc
        // drawn in a widget to the outer border of the widget
        //
        // padding of 1 space on left and right of a button:
        // [ text ]
        // padding of 4 space on left and right of a button:
        // [    text    ]
        Padding inner_padding{
            .top = 5.0f,
            .bottom = 5.0f,
            .left = 10.0f,
            .right = 10.0f,
        };

        // the gap of space measured from the outer border of each
        // widget to the border of the layout cell containing them
        //
        // margin of 1 on all sides of two buttons in vertical layout:
        // +----------+
        // | [ text ] |
        // +----------+
        // | [ text ] |
        // +----------+
        //
        // margin of 2 on all sides of two buttons in vertical layout:
        //
        // +------------+
        // |            |
        // |  [ text ]  |
        // |            |
        // +------------+
        // |            |
        // |  [ text ]  |
        // |            |
        // +------------+
        Margin outer_margin{
            .top = 10.0f,
            .bottom = 10.0f,
            .left = 10.0f,
            .right = 10.0f,
        };

        // how much a widget should stretch
        f32 stretch_factor{ 0.0f };

        // the layout cell's rectangle (debug only)
        ds::rect<f32> rect{ 0.0f, 0.0f, 0.0f, 0.0f };
    };

    class DynamicLayout : public ds::refcounted
    {
    public:
        constexpr explicit DynamicLayout() = default;

        virtual ~DynamicLayout() override
        {
            m_owner = nullptr;
        }

        // Performs applies all Layout computations for the given widget.
        virtual void apply_layout(u32 depth = 0) = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> computed_size() const = 0;

        auto widgets() const
        {
            return m_cell_data | std::ranges::views::keys;
        }

        void set_owner(Widget* widget)
        {
            m_owner = widget;
        }

    protected:
        Widget* m_owner{ nullptr };
        ds::rect<f32> m_layout_rect{ 0.0f, 0.0f, 0.0f, 0.0f };
        std::vector<std::pair<Widget*, CellProperties>> m_cell_data{};
        Margin outer_margin{
            .top = 10.0f,
            .bottom = 10.0f,
            .left = 10.0f,
            .right = 10.0f,
        };
        // Spacing m_spacing{};
    };

    class Layout : public ds::refcounted
    {
    public:
        // template <typename T>
        // struct Spacing
        //{
        //     T horizontal{};
        //     T vertical{};
        // };

    public:
        // Performs applies all Layout computations for the given widget.
        virtual void apply_layout(nvg::Context* nvc, const Widget* w) const = 0;
        virtual void apply_layout(nvg::Context* nvc, const Widget* w){};
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> computed_size(nvg::Context* nvc, const Widget* w) const = 0;
    };
}
