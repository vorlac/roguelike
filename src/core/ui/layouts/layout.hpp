#pragma once

#include "core/ui/widget.hpp"
#include "ds/margin.hpp"
#include "ds/refcounted.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;

    struct CellProperties
    {
        Placement_OldAlignment alignment{ Placement_OldAlignment::Fill };

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
        ds::margin<f32> outer_margin{ 20.0f, 20.0f, 20.0f, 20.0f };
        ds::margin<f32> inner_margin{ 20.0f, 20.0f, 20.0f, 20.0f };

        // how much a widget should stretch
        f32 stretch_factor{ 0.0f };

        // the layout cell's rectangle (debug only)
        ds::rect<f32> rect{ ds::rect<f32>::zero() };
    };

    class Layout : public Widget
    {
    public:
        explicit Layout(const std::string& name)
            : Widget(nullptr)
        {
            this->set_name(name);
            this->set_tooltip(name);

            m_layout = this;
        }

        void add_widget(Widget* widget, const CellProperties& properties = {
                                            .outer_margin = { 2.0f, 2.0f, 2.0f, 2.0f },
                                            .inner_margin = { 2.0f, 2.0f, 2.0f, 2.0f },
                                        })
        {
            this->add_child(widget);
            m_cell_data.emplace_back(widget, properties);
        }

        void add_nested_layout(Layout* layout, const CellProperties& properties = {
                                                   .outer_margin = { 2.0f, 2.0f, 2.0f, 2.0f },
                                                   .inner_margin = { 2.0f, 2.0f, 2.0f, 2.0f },
                                               })
        {
            this->add_widget(layout, properties);
        }

        auto widgets() const
        {
            return m_cell_data | std::ranges::views::keys;
        }

        void set_size_policy(const SizePolicy policy)
        {
            m_size_policy = policy;
        }

        SizePolicy size_policy() const
        {
            return m_size_policy;
        }

        Alignment alignment() const
        {
            return m_alignment;
        }

        const ds::margin<f32>& outer_margin() const
        {
            return m_outer_margin;
        }

        ds::margin<f32> inner_margin() const
        {
            return m_inner_margin;
        }

        void inner_margin(const ds::margin<f32> margin)
        {
            m_inner_margin = margin;
        }

        void outer_margin(const ds::margin<f32> margin)
        {
            m_outer_margin = margin;
        }

        void set_margins(const ds::margin<f32> inner, const ds::margin<f32> outer)
        {
            this->inner_margin(inner);
            this->inner_margin(outer);
        }

    public:
        // Performs applies all Layout computations for the given widget.
        virtual void apply_layout() = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> computed_size() const = 0;

    protected:
        Alignment m_alignment{ Alignment::None };
        SizePolicy m_size_policy{ SizePolicy::Prefered };
        std::vector<std::pair<Widget*, CellProperties>> m_cell_data{};
        ds::margin<f32> m_outer_margin{ ds::margin<f32>::init(5.0f) };
        ds::margin<f32> m_inner_margin{ ds::margin<f32>::init(5.0f) };
        ds::margin<f32> m_inner_padding{ ds::margin<f32>::init(5.0f) };
        f32 m_spacing{ 10.0f };
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
