#pragma once

#include <memory>

#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "ui/layouts/layout.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    template <Alignment VAlignment>
    class BoxLayout final : public Layout
    {
    public:
        constexpr explicit BoxLayout(const std::string& name)
            : Layout{ name }
        {
            m_alignment = VAlignment;
        }

    public:
        u64 columns() const
            requires rl::equal_v<VAlignment, Alignment::Horizontal>
        {
            return m_cell_data.size();
        }

        u64 rows() const
            requires rl::equal_v<VAlignment, Alignment::Vertical>
        {
            return m_cell_data.size();
        }

        virtual void adjust_for_size_policy() override
        {
            switch (this->size_policy()) {
                case SizePolicy::Minimum:
                {
                    Widget* parent{ this->parent() };
                    debug_assert(parent != nullptr,
                                 "layout must have a parent widget");

                    parent->set_size(m_rect.size + m_outer_margin);
                    break;
                }
                case SizePolicy::Maximum:
                {
                    const Widget* parent_widget{ this->parent() };
                    debug_assert(parent_widget != nullptr,
                                 "layout missing a parent");

                    const auto& siblings{ parent_widget->children() };
                    const Layout* parent_layout{ parent_widget->layout() };

                    const ds::dims<f32> fill_size{
                        parent_layout != nullptr
                            ? parent_layout->size() - m_outer_margin - parent_layout->inner_margin()
                            : parent_widget->size() - m_outer_margin
                    };

                    if (parent_layout == nullptr) {
                        debug_assert(siblings.size() == 1,
                                     "topmost layout must be an only child");
                        m_rect.size = fill_size;
                        m_rect.pt = m_outer_margin.offset();
                    }
                    else {
                        // not the topmost layout, so size will be decided
                        // by the amount of available space in parent layout
                        const Alignment parent_alignment{ parent_layout->alignment() };
                        const f32 sibling_count{ static_cast<f32>(siblings.size()) };

                        // calculate the combined size of self + siblings
                        ds::dims<f32> total_size{ ds::dims<f32>::zero() };
                        for (const Widget* sibling : siblings) {
                            const Layout* sibling_layout{ sibling->layout() };
                            const ds::margin<f32> sib_outer_margin{
                                sibling_layout != nullptr
                                    ? sibling_layout->outer_margin()
                                    : ds::margin<f32>::zero()
                            };

                            total_size += sibling->size() + sib_outer_margin;
                        }

                        const ds::dims<f32> delta_size{ fill_size - total_size };
                        const ds::dims<f32> size_increase{ delta_size / sibling_count };
                        for (auto [sibling_idx, sibling] : siblings | std::views::enumerate) {
                            ds::rect<f32> rect{ sibling->rect() };
                            const f32 sibling_order{ static_cast<f32>(sibling_idx) };
                            const Layout* sibling_layout{ sibling->layout() };
                            const ds::margin<f32> sib_outer{
                                sibling_layout != nullptr
                                    ? sibling_layout->outer_margin()
                                    : ds::margin<f32>::zero()
                            };

                            switch (parent_alignment) {
                                case Alignment::Horizontal:
                                    rect.pt.x += (size_increase.width + sib_outer.right) * sibling_order;
                                    rect.size.width += size_increase.width + sib_outer.right;
                                    rect.size.height = fill_size.height;
                                    break;
                                case Alignment::Vertical:
                                    rect.pt.y += (size_increase.height + sib_outer.bottom) * sibling_order;
                                    rect.size.height += size_increase.height + sib_outer.bottom;
                                    rect.size.width = fill_size.width;
                                    break;
                                case Alignment::None:
                                    debug_assert("invalid layout alignment");
                                    break;
                            }

                            sibling->set_rect(rect);
                        }
                    }

                    if (m_children.size() > 0) {
                        Layout* child_layout{ m_children.front()->layout() };
                        if (child_layout != nullptr)
                            child_layout->adjust_for_size_policy();
                        else {
                            // calculate combined size of all children
                            ds::dims<f32> total_size{ ds::dims<f32>::zero() };
                            for (const Widget* child : m_children)
                                total_size += child->size();

                            const f32 child_count{ static_cast<f32>(m_children.size()) };
                            ds::dims<f32> layout_fill_size{ m_rect.size - m_inner_margin - m_outer_margin };
                            if constexpr (VAlignment == Alignment::Horizontal)
                                layout_fill_size.width -= m_spacing * (child_count - 1.0f);
                            if constexpr (VAlignment == Alignment::Vertical)
                                layout_fill_size.height -= m_spacing * (child_count - 1.0f);

                            const ds::dims<f32> delta_size{ layout_fill_size - total_size };
                            const ds::dims<f32> size_increase{ delta_size / child_count };
                            for (auto [child_idx, child] : m_children | std::views::enumerate) {
                                const f32 child_order{ static_cast<f32>(child_idx) };

                                ds::rect<f32> rect{ child->rect() };
                                if constexpr (VAlignment == Alignment::Horizontal) {
                                    rect.pt.x += size_increase.width * child_order;
                                    rect.size.width += size_increase.width;
                                    rect.size.height = layout_fill_size.height;
                                }
                                if constexpr (VAlignment == Alignment::Vertical) {
                                    rect.pt.y += size_increase.height * child_order;
                                    rect.size.height += size_increase.height;
                                    rect.size.width = layout_fill_size.width;
                                }

                                child->set_rect(rect);
                            }
                        }
                    }

                    break;
                }

                case SizePolicy::FixedSize:
                    [[fallthrough]];
                case SizePolicy::Prefered:
                    break;
                case SizePolicy::Inherit:
                    debug_assert("layout must define a size policy");
                    break;
            }
        }

        virtual void apply_layout() override
        {
            ds::rect<f32> computed_rect{ ds::rect<f32>::zero() };
            ds::point<f32> curr_widget_pos{ m_inner_margin.offset() };

            for (Widget* widget : m_cell_data | std::views::keys) {
                ds::rect<f32> widget_rect{ ds::rect<f32>::zero() };
                Layout* widget_layout{ widget->layout() };
                if (widget_layout == nullptr) {
                    widget_rect.size = widget->preferred_size();
                    widget_rect.pt += curr_widget_pos + m_outer_margin.offset();
                }
                else {
                    widget_layout->apply_layout();
                    widget_rect = widget_layout->rect();
                    widget_rect.pt += curr_widget_pos;
                    widget_layout->set_rect(widget_rect);
                }

                widget->set_rect(widget_rect);
                computed_rect.expand(widget_rect);

                if constexpr (VAlignment == Alignment::Horizontal)
                    curr_widget_pos.x += widget_rect.size.width + m_spacing;
                else if constexpr (VAlignment == Alignment::Vertical)
                    curr_widget_pos.y += widget_rect.size.height + m_spacing;
            }

            computed_rect.pt += m_outer_margin.offset();
            computed_rect.size += ds::vector2{
                m_inner_margin.right + m_outer_margin.right,
                m_inner_margin.bottom + m_outer_margin.bottom,
            };

            this->set_rect(computed_rect);
        }

        virtual ds::dims<f32> computed_size() const override
        {
            ds::dims<f32> computed_size{ ds::dims<f32>::zero() };
            for (const Widget* widget : m_cell_data | std::views::keys) {
                const Layout* widget_layout{ widget->layout() };
                const ds::dims<f32> widget_computed_size{
                    widget_layout != nullptr
                        ? widget_layout->computed_size()
                        : widget->preferred_size()
                };

                switch (VAlignment) {
                    case Alignment::Horizontal:
                        computed_size.width += widget_computed_size.width;
                        if (math::equal(computed_size.height, 0.0f))
                            computed_size.height = widget_computed_size.height;
                        break;
                    case Alignment::Vertical:
                        computed_size.height += widget_computed_size.height;
                        if (math::equal(computed_size.width, 0.0f))
                            computed_size.width = widget_computed_size.width;
                        break;
                    case Alignment::None:
                        debug_assert("Invalid layout alignment: {}", VAlignment);
                        break;
                }

                computed_size += m_inner_margin;
                computed_size += m_outer_margin;
            }

            return computed_size;
        }
    };
}
