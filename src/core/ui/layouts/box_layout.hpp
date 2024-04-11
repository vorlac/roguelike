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

    template <Alignment VOrientation>
    class BoxLayout final : public Layout
    {
    public:
        // using Layout::arrangement;
        constexpr explicit BoxLayout(const std::string& name)
            : Layout{ name }
        {
            m_alignment = VOrientation;
        }

    public:
        u64 columns() const
            requires std::same_as<VOrientation, Alignment::Horizontal>
        {
            return m_cell_data.size();
        }

        u64 rows() const
            requires std::same_as<VOrientation, Alignment::Vertical>
        {
            return m_cell_data.size();
        }

        virtual void apply_layout(const u32 depth = 0) override
        {
            SizePolicy size_policy{ this->size_policy() };
            ds::point curr_widget_pos{ ds::point<f32>::zero() };
            ds::dims cell_size{ m_max_size };

            ds::rect<f32> layout_rect{ m_rect };

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
                ds::rect widget_bounding_rect{ ds::rect<f32>::zero() };
                ds::rect widget_margins_rect{ ds::rect<f32>::zero() };

                if (size_policy == SizePolicy::Prefered)
                {
                    widget_margins_rect = ds::rect{
                        curr_widget_pos +
                            ds::vector2{
                                m_inner_margin.left,
                                m_inner_margin.top,
                            },
                        cell_size - m_inner_margin,
                    };

                    widget_bounding_rect = ds::rect{
                        widget_margins_rect.pt +
                            ds::vector2{
                                props.outer_margin.left,
                                props.outer_margin.top,
                            },
                        widget_margins_rect.size - props.outer_margin,
                    };
                }
                else
                {
                    // create the widget's rect (position and size) based on it's
                    // relative position to the parent widget's top-left corner,
                    // which would be (0,0) in the current local coordinate system
                    const ds::dims widget_pref_size{ widget->preferred_size() };
                    const ds::dims widget_fixed_size{ widget->fixed_size() };
                    const ds::dims widget_actual_size{ widget_fixed_size.merged(widget_pref_size) };

                    widget_bounding_rect = ds::rect<f32>{
                        curr_widget_pos +
                            ds::vector2<f32>{
                                m_inner_margin.left,
                                m_inner_margin.top,
                            },
                        widget_actual_size,
                    };

                    widget_margins_rect = widget_bounding_rect.expanded(props.outer_margin);
                }

                if (size_policy == SizePolicy::Prefered)
                {
                    if (m_alignment == Alignment::Vertical)
                        curr_widget_pos.y += widget_margins_rect.size.height +
                                             props.outer_margin.vertical();
                    else if (m_alignment == Alignment::Horizontal)
                        curr_widget_pos.x += widget_margins_rect.size.width +
                                             props.outer_margin.horizontal();
                }
                else
                {
                    // compute the rect representing the widget's rect including external margins
                    if (m_alignment == Alignment::Vertical)
                    {
                        // assign positions from top to bottom
                        f32 height_offset{ widget_bounding_rect.size.height +
                                           props.outer_margin.vertical() };
                        layout_rect.size.height += height_offset;
                        curr_widget_pos.y += height_offset;
                    }

                    if (m_alignment == Alignment::Horizontal)
                    {
                        // assign positions from top to bottom
                        f32 width_offset{ widget_bounding_rect.size.width +
                                          props.outer_margin.horizontal() };
                        layout_rect.size.width += width_offset;
                        curr_widget_pos.x += width_offset;
                    }

                    // offset the widget's local position, relative to
                    // parent layout widget, by it's top & left margin
                    widget_bounding_rect.pt.x += m_outer_margin.left;
                    widget_bounding_rect.pt.y += m_outer_margin.top;
                }

                // assign the current child's position and dimensions
                widget->set_rect(widget_bounding_rect);

                // grow the current layout's position and dimensions
                // so that it includes the newly computed child rect
                layout_rect.expand(widget_margins_rect);
            }

            if (size_policy != SizePolicy::Prefered)
            {
                //  increase the total dimensions by the layout widget's
                //  absolute vertical and hotizontal outer margin values
                layout_rect.size.width += m_outer_margin.vertical();
                layout_rect.size.height += m_outer_margin.horizontal();
            }

            // assign the newly computed rect for the layout widget
            this->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override
        {
            return this->size();
        }
    };
}
