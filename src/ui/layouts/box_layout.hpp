#pragma once

#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "ui/layouts/layout.hpp"
#include "utils/properties.hpp"

namespace rl::ui {
    template <Alignment VOrientation>
    class BoxLayout final : public Layout
    {
    public:
        constexpr explicit BoxLayout(const std::string& name)
            : Layout{ name }
        {
            m_alignment = VOrientation;
        }

    public:
        u64 columns() const
            requires rl::equal_v<VOrientation, Alignment::Horizontal>
        {
            return m_cell_data.size();
        }

        u64 rows() const
            requires rl::equal_v<VOrientation, Alignment::Vertical>
        {
            return m_cell_data.size();
        }

        virtual void adjust_for_size_policy() override
        {
            switch (m_layout->size_policy()) {
                case SizePolicy::Minimum:
                {
                    Widget* parent{ m_layout->parent() };
                    runtime_assert(parent != nullptr, "layout must have a parent widget");
                    parent->set_size(m_layout->size() + m_layout->outer_margin());
                    break;
                }
                case SizePolicy::Maximum:
                {
                    const Widget* parent{ m_layout->parent() };
                    runtime_assert(parent != nullptr, "layout must have a parent widget");

                    const Layout* parent_layout{ parent->layout() };
                    const ds::dims<f32> fill_size{
                        parent_layout != nullptr
                            ? parent_layout->size()
                                  - parent_layout->inner_margin()
                                  - m_layout->outer_margin()
                            : parent->size()
                                  - m_layout->outer_margin()
                    };

                    const f32 sibling_count{ static_cast<f32>(parent->child_count()) };
                    if constexpr (VOrientation == Alignment::Horizontal) {
                        const f32 delta_width{ fill_size.width - m_layout->width() };
                        const f32 width_increase{ delta_width / sibling_count };
                        m_rect.size.width += width_increase;
                    }
                    else if constexpr (VOrientation == Alignment::Vertical) {
                        const f32 delta_width{ fill_size.width - m_layout->width() };
                        const f32 width_increase{ delta_width / sibling_count };
                        m_rect.size.width += width_increase;
                    }

                    for (const Widget* child : m_children) {
                        Layout* child_layout{ child->layout() };
                        if (child_layout != nullptr)
                            child_layout->adjust_for_size_policy();
                    }

                    break;
                }
                case SizePolicy::Prefered:
                {
                    break;
                }
                default:
                    break;
            }
        }

        virtual void apply_layout() override
        {
            ds::rect<f32> computed_rect{ ds::rect<f32>::zero() };
            ds::point<f32> curr_widget_pos{ m_inner_margin.offset() };

            for (auto&& widget : m_cell_data | std::views::keys) {
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

                if constexpr (VOrientation == Alignment::Horizontal)
                    curr_widget_pos.x += widget_rect.size.width + m_spacing;
                else if constexpr (VOrientation == Alignment::Vertical)
                    curr_widget_pos.y += widget_rect.size.height + m_spacing;
            }

            computed_rect.pt += m_outer_margin.offset();
            computed_rect.size += ds::vector2{
                m_inner_margin.right + m_outer_margin.right,
                m_inner_margin.bottom + m_outer_margin.bottom,
            };
            this->set_rect(computed_rect);
            Widget::set_rect(computed_rect);
        }

        virtual ds::dims<f32> computed_size() const override
        {
            ds::dims<f32> computed_size{ ds::dims<f32>::zero() };
            for (auto& widget : m_cell_data | std::views::keys) {
                const Layout* widget_layout{ widget->layout() };
                const ds::dims<f32> widget_computed_size{
                    widget_layout != nullptr
                        ? widget_layout->computed_size()
                        : widget->preferred_size(),
                };

                switch (VOrientation) {
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
                        assert_msg("Invalid layout alignment: {}", VOrientation);
                        break;
                }

                computed_size += m_inner_margin;
                computed_size += m_outer_margin;
            }

            return computed_size;
        }
    };
}
