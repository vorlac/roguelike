#pragma once

#include "core/ui/layouts/layout.hpp"
#include "core/ui/widgets/panel.hpp"
#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/properties.hpp"

namespace rl::nvg {
    struct Context;
}

namespace rl::ui {
    class Widget;
    class Panel;

    template <Arrangement VOrientation>
    class BoxLayout final : public Layout
    {
    public:
        constexpr static inline Arrangement orientation = VOrientation;
        using Layout::Layout;

    public:
        u64 columns() const
            requires std::same_as<VOrientation, Arrangement::Horizontal>
        {
            return m_cell_data.size();
        }

        u64 rows() const
            requires std::same_as<VOrientation, Arrangement::Vertical>
        {
            return m_cell_data.size();
        }

        virtual void apply_layout(const u32 depth = 0) override
        {
            ds::point curr_widget_pos{ ds::point<f32>::zero() };
            ds::rect layout_rect{ ds::rect<f32>::zero() };
            ds::dims cell_size{ ds::dims<f32>::zero() };

            const u64 cell_count{ m_cell_data.size() };
            if constexpr (orientation == Arrangement::Vertical)
            {
                cell_size.width = m_max_size.width;
                cell_size.height = m_max_size.height / cell_count;
            }
            if constexpr (orientation == Arrangement::Horizontal)
            {
                cell_size.width = m_max_size.width / cell_count;
                cell_size.height = m_max_size.height;
                cell_size -= m_outer_margin;
            }

            auto size_policy{ this->size_policy() };
            // first process and/or recompute the sizes of
            // each cell that contains a widget in the layout
            for (auto&& [widget, props] : m_cell_data)
            {
                // for each layout widget, recursively update the sizes of each leaf
                // node widget in the tree below the current widget being processed
                for (Widget* child : widget->children())
                {
                    Layout* child_layout{ child->layout() };
                    if (child_layout != nullptr)
                        child_layout->apply_layout(depth + 1);
                }

                // compute the widget's layout cell position and size
                ds::rect<f32> widget_bounding_rect{};
                ds::rect<f32> widget_margins_rect{};

                if (size_policy == SizePolicy::Prefered)
                {
                    widget_bounding_rect = ds::rect{
                        curr_widget_pos +
                            ds::vector2{
                                props.outer_margin.left,
                                props.outer_margin.top,
                            },
                        cell_size,
                    };

                    widget_margins_rect = widget_bounding_rect;
                }
                else
                {
                    // create the widget's rect (position and size) based on it's
                    // relative position to the parent widget's top-left corner,
                    // which would be (0,0) in the current local coordinate system
                    const ds::dims widget_pref_size{ widget->preferred_size() };
                    const ds::dims widget_fixed_size{ widget->fixed_size() };
                    const ds::dims widget_actual_size{ widget_fixed_size.merged(widget_pref_size) };

                    widget_bounding_rect = ds::rect{
                        curr_widget_pos +
                            ds::vector2{
                                props.outer_margin.left,
                                props.outer_margin.top,
                            },
                        widget_actual_size,
                    };

                    widget_margins_rect = widget_bounding_rect.expanded(props.outer_margin);
                }
                // compute the rect representing the widget's rect including external margins
                if constexpr (orientation == Arrangement::Vertical)
                {
                    if (size_policy == SizePolicy::Prefered)
                    {
                        // const f32 height_offset{ (cell_height / 2.0f) -
                        //                          (widget_actual_size.height / 2.0f) };

                        // assign positions from top to bottom
                        layout_rect.size.height += cell_size.height;
                        curr_widget_pos.y += cell_size.height;
                    }
                    else
                    {
                        const f32 height_offset{ widget_bounding_rect.size.height +
                                                 props.outer_margin.vertical() };

                        // assign positions from top to bottom
                        layout_rect.size.height += height_offset;
                        curr_widget_pos.y += height_offset;
                    }
                }

                if constexpr (orientation == Arrangement::Horizontal)
                {
                    if (size_policy == SizePolicy::Prefered)
                    {
                        // const f32 width_offset{ (cell_width / 2.0f) -
                        //                         (widget_actual_size.height / 2.0f) };

                        // assign positions from top to bottom
                        layout_rect.size.width += cell_size.width;
                        curr_widget_pos.x += cell_size.width;
                    }
                    else
                    {
                        const f32 width_offset{ widget_bounding_rect.size.width +
                                                props.outer_margin.horizontal() };

                        // assign positions from left to right
                        layout_rect.size.width += width_offset;
                        curr_widget_pos.x += width_offset;
                    }
                }

                if (size_policy != SizePolicy::Prefered)
                {
                    // offset the widget's local position, relative to
                    // parent layout widget, by it's top & left margin
                    widget_bounding_rect.pt.x += m_outer_margin.left;
                    widget_bounding_rect.pt.y += m_outer_margin.top;
                }

                // assign the current child's position and dimensions
                widget->set_rect(std::move(widget_bounding_rect));

                if (size_policy != SizePolicy::Prefered)
                {
                    // grow the current layout's position and dimensions
                    // so that it includes the newly computed child rect
                    layout_rect.expand(widget_margins_rect);
                }
            }

            if (size_policy == SizePolicy::Prefered)
            {
                layout_rect.pt += ds::vector2{ m_outer_margin.left, m_outer_margin.top };
                layout_rect.size += cell_size;
                //  this->set_rect(std::move(layout_rect));
            }
            else
            {
                // increase the total dimensions by the layout widget's
                // absolute vertical and hotizontal outer margin values
                layout_rect.size.width += m_outer_margin.vertical();
                layout_rect.size.height += m_outer_margin.horizontal();
                // assign the newly computed rect for the layout widget
            }

            this->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override
        {
            return this->size();
        }
    };
}
