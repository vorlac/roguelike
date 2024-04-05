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
            ds::point curr_widget_pos{ 0.0f, 0.0f };
            ds::rect layout_rect{ 0.0f, 0.0f, 0.0f, 0.0f };

            // first process and/or recompute the sizes of
            // each cell that contains a widget in the layout
            for (auto&& [widget, props] : m_cell_data)
            {
                // for each layout widget, recursively update the sizes of each leaf
                // node widget in the tree below the current widget being processed
                for (Widget* child : widget->children())
                {
                    auto child_layout{ child->layout() };
                    if (child_layout != nullptr)
                        child_layout->apply_layout(depth + 1);
                }

                // create the widget's rect (position and size) based on it's
                // relative position to the parent widget's top-left corner,
                // which would be (0,0) in the current local coordinate system
                const ds::dims widget_pref_size{ widget->preferred_size() };
                const ds::dims widget_fixed_size{ widget->fixed_size() };
                const ds::dims widget_actual_size{ widget_fixed_size.merged(widget_pref_size) };

                // compute the widget's layout cell position and size
                ds::rect widget_bounding_rect{
                    curr_widget_pos +
                        ds::vector2{
                            props.outer_margin.left,
                            props.outer_margin.top,
                        },
                    widget_actual_size,
                };

                // compute the rect representing the widget's rect including external margins
                ds::rect widget_margins_rect{ widget_bounding_rect.expanded(props.outer_margin) };

                if constexpr (orientation == Arrangement::Vertical)
                {
                    f32 height_offset{ widget_bounding_rect.size.height +
                                       props.outer_margin.vertical() };

                    // assign positions from top to bottom
                    layout_rect.size.height += height_offset;
                    curr_widget_pos.y += height_offset;
                }

                if constexpr (orientation == Arrangement::Horizontal)
                {
                    f32 width_offset{ widget_bounding_rect.size.width +
                                      props.outer_margin.horizontal() };

                    // assign positions from left to right
                    layout_rect.size.width += width_offset;
                    curr_widget_pos.x += width_offset;
                }

                // offset the widget's local position, relative to
                // parent layout widget, by it's top & left margin
                widget_bounding_rect.pt.x += m_outer_margin.left;
                widget_bounding_rect.pt.y += m_outer_margin.top;

                // assign the current child's position and dimensions
                widget->set_rect(std::move(widget_bounding_rect));

                // grow the current layout's position and dimensions
                // so that it includes the newly computed child rect
                layout_rect.expand(widget_margins_rect);
            }

            // increase the total dimensions by the layout widget's
            // absolute vertical and hotizontal outer margin values
            layout_rect.size.width += m_outer_margin.vertical();
            layout_rect.size.height += m_outer_margin.horizontal();

            // assign the newly computed rect for the layout widget
            this->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override
        {
            return this->size();
        }
    };
}
