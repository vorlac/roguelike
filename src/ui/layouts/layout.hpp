#pragma once

#include "ds/margin.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "gfx/vg/nanovg.hpp"
#include "ui/widget.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    class Widget;

    struct CellProperties
    {
        f32 stretch_factor{ 0.0f };
        Placement_OldAlignment alignment{ Placement_OldAlignment::Fill };
        ds::margin<f32> inner_padding{ 5.0f, 5.0f, 10.0f, 10.0f };
        ds::margin<f32> outer_margin{ 20.0f, 20.0f, 20.0f, 20.0f };
        ds::margin<f32> inner_margin{ 20.0f, 20.0f, 20.0f, 20.0f };
    };

    class Layout : public Widget
    {
    public:
        explicit Layout(std::string name)
            : Widget(nullptr)
        {
            this->set_name(std::move(name));
            // TODO: remove, debug
            this->set_tooltip(std::string{ this->name() });
            // this just bypasses the
            // need for dynamic_cast
            m_layout = this;
        }

        void add_widget(Widget* widget)
        {
            this->add_child(widget);
            m_cell_data.emplace_back(widget, CellProperties{});
        }

        void add_nested_layout(Layout* layout)
        {
            this->add_widget(layout);
            if (m_size_policy != SizePolicy::Inherit &&
                layout->size_policy() == SizePolicy::Inherit)
                layout->set_size_policy(m_size_policy);
        }

        void set_size_policy(const SizePolicy policy)
        {
            if (m_size_policy == SizePolicy::Inherit && policy != SizePolicy::Inherit) {
                for (const Widget* widget : m_cell_data | std::views::keys) {
                    Layout* widget_layout{ widget->layout() };
                    if (widget_layout != nullptr)
                        widget_layout->set_size_policy(policy);
                }
            }

            m_size_policy = policy;
        }

        void set_inner_margin(const ds::margin<f32> margin)
        {
            m_inner_margin = margin;
        }

        void set_outer_margin(const ds::margin<f32> margin)
        {
            m_outer_margin = margin;
        }

        void set_margins(const ds::margin<f32> inner, const ds::margin<f32> outer)
        {
            this->set_inner_margin(inner);
            this->set_outer_margin(outer);
        }

        [[nodiscard]]
        auto widgets() const
        {
            return m_cell_data | std::ranges::views::keys;
        }

        [[nodiscard]]
        SizePolicy size_policy() const
        {
            return m_size_policy;
        }

        [[nodiscard]]
        Alignment alignment() const
        {
            return m_alignment;
        }

        [[nodiscard]]
        ds::margin<f32> outer_margin() const
        {
            return m_outer_margin;
        }

        [[nodiscard]]
        ds::margin<f32> inner_margin() const
        {
            return m_inner_margin;
        }

    public:
        // Performs all Layout computations for the given widget.
        virtual void apply_layout() = 0;
        // update contents of layout based on it's size policy
        virtual void adjust_for_size_policy() = 0;
        // Compute the preferred size for a given Layout and widget
        virtual ds::dims<f32> computed_size() const = 0;

    protected:
        Alignment m_alignment{ Alignment::None };
        SizePolicy m_size_policy{ SizePolicy::Inherit };
        std::vector<std::pair<Widget*, CellProperties>> m_cell_data{};
        ds::margin<f32> m_outer_margin{ ds::margin<f32>::init(3.0f) };
        ds::margin<f32> m_inner_margin{ ds::margin<f32>::init(3.0f) };
        // spacing between widgets managed by layout
        f32 m_spacing{ 5.0f };
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
