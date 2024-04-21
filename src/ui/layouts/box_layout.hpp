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
                    runtime_assert(parent != nullptr,
                                   "layout must have a parent widget");

                    parent->set_size(m_rect.size + m_outer_margin);
                    break;
                }

                case SizePolicy::Maximum:
                {
                    const Widget* parent_widget{ this->parent() };
                    runtime_assert(parent_widget != nullptr,
                                   "layout missing a parent");

                    ds::dims<f32> total_size{ ds::dims<f32>::zero() };
                    const auto& siblings{ parent_widget->children() };
                    const Layout* parent_layout{ parent_widget->layout() };

                    const ds::dims<f32> fill_size{
                        parent_layout != nullptr
                            ? parent_layout->size() - m_outer_margin - parent_layout->inner_margin()
                            : parent_widget->size() - m_outer_margin
                    };

                    if (parent_layout == nullptr) {
                        runtime_assert(siblings.size() == 1,
                                       "topmost layout must be an only child");
                        m_rect.size = fill_size;
                        m_rect.pt = m_outer_margin.offset();
                    }
                    else {
                        // not the topmost layout, so size will be decided
                        // by the amount of available space in parent layout
                        const Alignment parent_alignment{ parent_layout->alignment() };
                        const f32 sibling_count{ static_cast<f32>(siblings.size()) };
                        auto& magnitude{ parent_alignment == Alignment::Vertical
                                             ? total_size.height
                                             : total_size.width };

                        // calculate the total length of all siblings (including this one)
                        // in the dimension that the parent layout aligns it's contents
                        for (const Widget* sibling : siblings) {
                            const Layout* sibling_layout{ sibling->layout() };
                            ds::margin<f32> sib_outer_margin{
                                sibling_layout != nullptr
                                    ? sibling_layout->outer_margin()
                                    : ds::margin<f32>::zero()
                            };

                            magnitude += parent_alignment == Alignment::Vertical
                                           ? sibling->height() + sib_outer_margin.vertical()
                                           : sibling->width() + sib_outer_margin.horizontal();
                        }

                        // TODO:
                        // TODO: Need to handle the sibling alignment..
                        // TODO: NOT the current widget's alignment here
                        // TODO:
                        if constexpr (VAlignment == Alignment::Horizontal) {
                            const f32 delta_height{ fill_size.height - magnitude };
                            const f32 height_increase{ delta_height / sibling_count };
                            for (auto [sibling_idx, sibling] : siblings | std::views::enumerate) {
                                const Layout* sibling_layout{ sibling->layout() };
                                const ds::margin<f32> sib_outer{
                                    sibling_layout != nullptr
                                        ? sibling_layout->outer_margin()
                                        : ds::margin<f32>::zero()
                                };

                                ds::rect<f32> rect{ sibling->rect() };
                                rect.pt.y += (height_increase + sib_outer.bottom) * static_cast<f32>(sibling_idx);
                                rect.size.height += height_increase + sib_outer.bottom;
                                if (parent_alignment == Alignment::Vertical) {
                                    // no siblings to worry about when stretching
                                    // horizontally since parent aligns vertically
                                    rect.size.width = fill_size.width;
                                }
                                else {
                                    int a = 123;
                                    (void)a;
                                    // TODO: handle width expansion for nested horizontal layouts
                                }

                                sibling->set_rect(rect);
                            }
                        }

                        // TODO:
                        // TODO: Need to handle the sibling alignment..
                        // TODO: NOT the current widget's alignment here
                        // TODO:
                        if constexpr (VAlignment == Alignment::Vertical) {
                            const f32 delta_width{ fill_size.width - magnitude };
                            const f32 width_increase{ delta_width / sibling_count };
                            for (auto [sibling_idx, sibling] : siblings | std::views::enumerate) {
                                const Layout* sibling_layout{ sibling->layout() };
                                const ds::margin<f32> sib_outer{
                                    sibling_layout != nullptr
                                        ? sibling_layout->outer_margin()
                                        : ds::margin<f32>::zero()
                                };

                                ds::rect<f32> rect{ sibling->rect() };
                                rect.pt.x += (width_increase + sib_outer.right) * static_cast<f32>(sibling_idx);
                                rect.size.width += width_increase + sib_outer.right;
                                if (parent_alignment == Alignment::Horizontal) {
                                    // no siblings to worry about when stretching
                                    // vertically since parent aligns horizontally
                                    rect.size.height = fill_size.height;
                                }
                                else {
                                    int a = 123;
                                    (void)a;
                                    // TODO: handle width expansion for nested horizontal layouts
                                }

                                sibling->set_rect(rect);
                            }
                        }
                    }

                    if (m_children.size() > 0) {
                        // the recursive call only needs to happen
                        // for the first child because its size is
                        // recalculated along with all of the direct
                        // siblings contained in the parent layout
                        Widget* child{ m_children.front() };
                        Layout* child_layout{ child->layout() };
                        if (child_layout != nullptr)
                            child_layout->adjust_for_size_policy();
                    }

                    break;
                }

                case SizePolicy::FixedSize:
                    [[fallthrough]];
                case SizePolicy::Prefered:
                    break;
                case SizePolicy::Inherit:
                    assert_msg("layout must define a size policy");
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
            for (Widget* widget : m_cell_data | std::views::keys) {
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
                    default:
                    case Alignment::None:
                        assert_msg("Invalid layout alignment: {}", VAlignment);
                        break;
                }

                computed_size += m_inner_margin;
                computed_size += m_outer_margin;
            }

            return computed_size;
        }
    };
}
