#pragma once

#include <ranges>
#include <utility>

#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "ui/layouts/layout.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    template <Alignment VAlignment>
    class BoxLayout final : public Layout
    {
    public:
        constexpr explicit BoxLayout(std::string name)
            : Layout{ std::move(name) }
        {
            m_alignment = VAlignment;
        }

        constexpr explicit BoxLayout(std::string name, const std::initializer_list<Widget*>& widgets)
            : BoxLayout{ std::move(name) }
        {
            for (Widget* widget : widgets)
                this->add_widget(widget);
        }

        constexpr explicit BoxLayout(std::string name, const std::initializer_list<Layout*>& nested_layouts)
            : BoxLayout{ std::move(name) }
        {
            for (Layout* layout : nested_layouts)
                this->add_nested_layout(layout);
        }

        virtual void adjust_for_size_policy() override
        {
            switch (this->size_policy()) {
                case SizePolicy::Minimum:
                {
                    // interestingly enough, both Minimim and Maximum both
                    // use the same code to adjust internal layours/widgets.
                    // The main difference between the two is in an outer
                    // scope, where Minimim clamps the window's min & max
                    // size to perfectly fit the gui contents before anything
                    // is expanded, which is the only thing preventing the
                    // minimum policy to behave exactly like the maximum policy
                    [[fallthrough]];
                }
                case SizePolicy::Maximum:
                {
                    const Widget* parent_widget{ this->parent() };
                    debug_assert(parent_widget != nullptr,
                                 "layout missing a parent");

                    const auto& siblings{ parent_widget->children() };
                    const Layout* parent_layout{ parent_widget->layout() };

                    // first determine how much space is available for this layout and it's siblings
                    // by subtracting it's outer margin from the parent widget/layout's size
                    const ds::dims<f32> fill_size{
                        parent_layout != nullptr
                            ? parent_layout->size() - m_outer_margin - parent_layout->inner_margin()
                            : parent_widget->size() - m_outer_margin
                    };

                    debug_assert(fill_size.valid(), "dimensions must be positive");

                    if (parent_layout == nullptr) {
                        debug_assert(siblings.size() == 1, "root layout must be only child");
                        m_rect.pt = m_outer_margin.offset();
                        m_rect.size = fill_size;
                    }
                    else {
                        // not the topmost layout, so size will be decided
                        // by the amount of available space in parent layout
                        // first, calculate the combined size of all siblings
                        ds::dims combined_size{ parent_layout->inner_margin() };
                        f32 combined_stretch{ 0.0f };
                        f32 expand_count{ 0.0f };

                        for (const Widget* sibling : siblings) {
                            const Layout* sibling_layout{ sibling->layout() };
                            combined_size += sibling->size();
                            if (sibling_layout == nullptr)
                                continue;

                            if (sibling_layout->size_policy() == SizePolicy::Maximum) {
                                combined_stretch += sibling->expansion();
                                expand_count += 1.0f;
                            }
                        }

                        const ds::dims<f32> delta_size{ fill_size - combined_size };
                        const ds::dims<f32> size_increase{ delta_size / combined_stretch };
                        const Alignment parent_alignment{ parent_layout->alignment() };

                        f32 siblings_expanded{ 0.0f };
                        for (auto [sibling_idx, sibling] : siblings | std::views::enumerate) {
                            const Layout* sibling_layout{ sibling->layout() };
                            ds::rect rect{ sibling_layout->rect() };

                            switch (parent_alignment) {
                                case Alignment::Horizontal:
                                    rect.size.height = fill_size.height;
                                    if (sibling_layout->size_policy() != SizePolicy::Minimum) {
                                        const f32 width_expansion{
                                            sibling_layout->expansion() *
                                            size_increase.width
                                        };

                                        rect.size.width += width_expansion;
                                        rect.pt.x += width_expansion *
                                                     siblings_expanded++;
                                    }
                                    break;

                                case Alignment::Vertical:
                                    rect.size.width = fill_size.width;
                                    if (sibling_layout->size_policy() != SizePolicy::Minimum) {
                                        const f32 height_expansion{
                                            sibling_layout->expansion() *
                                            size_increase.height
                                        };

                                        rect.size.height += height_expansion;
                                        rect.pt.y += height_expansion *
                                                     siblings_expanded++;
                                    }
                                    break;

                                case Alignment::None:
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
                            f32 combined_stretch{ 0.0f };
                            ds::dims<f32> children_combined_size{ 0.0f };

                            // calculate combined size of all children
                            for (const Widget* child : m_children) {
                                combined_stretch += child->expansion();
                                children_combined_size += child->size();
                            }

                            // calculate the inner bounds of this layout that should perfectly
                            // contain all children with spacing and size expansion applied evenly
                            ds::dims layout_fill_size{ m_rect.size - m_inner_margin - m_outer_margin };
                            const f32 spacing_count{ static_cast<f32>(m_children.size() - 1) };
                            if constexpr (VAlignment == Alignment::Horizontal)
                                layout_fill_size.width -= spacing_count * m_spacing;
                            if constexpr (VAlignment == Alignment::Vertical)
                                layout_fill_size.height -= spacing_count * m_spacing;

                            ds::vector2<f32> prev_offset{ m_inner_margin.horizontal(),
                                                          m_inner_margin.vertical() };

                            const ds::dims delta_size{ layout_fill_size - children_combined_size };
                            const ds::dims growth_expansion{ delta_size / combined_stretch };
                            for (auto [child_idx, child] : m_children | std::views::enumerate) {
                                const ds::dims<f32> actual_increase{
                                    growth_expansion * child->expansion()
                                };

                                ds::rect<f32> rect{ child->rect() };
                                if constexpr (VAlignment == Alignment::Horizontal) {
                                    rect.pt.x = prev_offset.x;
                                    rect.size.height = layout_fill_size.height;
                                    rect.size.width += actual_increase.width;
                                    prev_offset.x = rect.right() + m_spacing;
                                }
                                if constexpr (VAlignment == Alignment::Vertical) {
                                    rect.pt.y = prev_offset.y;
                                    rect.size.width = layout_fill_size.width;
                                    rect.size.height += actual_increase.height;
                                    prev_offset.y = rect.bottom() + m_spacing;
                                }

                                child->set_rect(rect);
                            }
                        }
                    }

                    break;
                }

                case SizePolicy::Freeform:
                    break;
                case SizePolicy::Inherit:
                    debug_assert("layout must define a size policy");
                    break;
            }
        }

        virtual void apply_layout() override
        {
            ds::rect<f32> computed_rect{};
            ds::point<f32> curr_widget_pos{ m_outer_margin.offset() };

            for (auto [idx, widget] : m_children | std::views::enumerate) {
                ds::rect<f32> widget_rect{};
                Layout* widget_layout{ widget->layout() };
                if (widget_layout == nullptr) {
                    widget_rect.size = widget->preferred_size();
                    widget_rect.pt += m_inner_margin.offset() + curr_widget_pos;
                }
                else {
                    widget_layout->apply_layout();
                    widget_rect = widget_layout->rect();
                    widget_rect.pt += curr_widget_pos;
                    widget_layout->set_rect(widget_rect);
                }

                widget->set_rect(widget_rect);
                computed_rect.engulf(widget_rect);

                if constexpr (VAlignment == Alignment::Horizontal) {
                    curr_widget_pos.x += widget_rect.size.width;
                    if (widget_layout == nullptr)
                        curr_widget_pos.x += m_spacing;
                    else {
                        const ds::margin widget_outer{ widget_layout->outer_margin() };
                        curr_widget_pos.x += widget_outer.horizontal();
                    }
                }
                if constexpr (VAlignment == Alignment::Vertical) {
                    curr_widget_pos.y += widget_rect.size.height;
                    if (widget_layout == nullptr)
                        curr_widget_pos.y += m_spacing;
                    else {
                        const ds::margin widget_outer{ widget_layout->outer_margin() };
                        curr_widget_pos.y += widget_outer.vertical();
                    }
                }
            }

            computed_rect.pt += m_outer_margin.offset();
            computed_rect.size += m_outer_margin;
            this->set_rect(computed_rect);
        }

        [[nodiscard]]
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
