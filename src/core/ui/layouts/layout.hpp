#pragma once

#include "core/ui/widgets/panel.hpp"
#include "ds/margin.hpp"
#include "ds/refcounted.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;

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
        ds::margin<f32> inner_padding{ 5.0f, 5.0f, 10.0f, 10.0f };

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
        ds::margin<f32> outer_margin{ 10.0f, 10.0f, 10.0f, 10.0f };

        // how much a widget should stretch
        f32 stretch_factor{ 0.0f };

        // the layout cell's rectangle (debug only)
        ds::rect<f32> rect{ 0.0f, 0.0f, 0.0f, 0.0f };
    };

    class Layout : public ds::refcounted
    {
    public:
        constexpr Layout() = default;

        constexpr virtual ~Layout() override
        {
            m_layout_panel = nullptr;
        }

        Widget* owner() const
        {
            return m_layout_owner;
        }

        void set_owner(Widget* widget)
        {
            m_layout_owner = widget;
        }

        Panel* layout_panel() const
        {
            return m_layout_panel;
        }

        void add_widget(Widget* widget, CellProperties properties)
        {
            m_layout_panel->add_child(widget);
            m_cell_data.emplace_back(widget, std::move(properties));
        }

        void add_widget(Widget* widget)
        {
            this->add_widget(widget, {});
        }

        void add_nested_layout(Layout* layout, CellProperties properties)
        {
            // assign the nested layout
            m_layout_panel->assign_layout(layout);
            // add the nested inner layout's panel as the widget being arranged
            // by the one it was passed into (`this` in the current call's scope).
            this->add_widget(layout->layout_panel(), std::move(properties));
        }

        auto widgets() const
        {
            return m_cell_data | std::ranges::views::keys;
        }

    public:
        // Performs applies all Layout computations for the given widget.
        virtual void apply_layout(u32 depth = 0) = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> computed_size() const = 0;

    protected:
        // the parent widget this layout will be arranging
        Widget* m_layout_owner{ nullptr };

        // empty panel widget used as a simple container
        // for widgets being managed by this layout
        Panel* m_layout_panel{ new Panel{ nullptr } };

        std::vector<std::pair<Widget*, CellProperties>> m_cell_data{};
        ds::margin<f32> m_outer_margin{ 10.0f, 10.0f, 10.0f, 10.0f };
    };

    class OldLayout : public ds::refcounted
    {
    public:
        // Performs applies all Layout computations for the given widget.
        virtual void apply_layout(nvg::Context* nvc, const Widget* w) const = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> computed_size(nvg::Context* nvc, const Widget* w) const = 0;
    };
}
