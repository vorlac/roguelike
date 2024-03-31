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
        friend class Widget;
        constexpr static inline Arrangement orientation = VOrientation;

    public:
        constexpr BoxLayout() = default;

        constexpr explicit BoxLayout(std::string&& name)
        {
            m_layout_panel->set_name(name);
            m_layout_panel->set_tooltip(name);
        }

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

                widget->set_rect(std::move(widget_bounding_rect));
                layout_rect.expand(widget_margins_rect);
            }

            // curr_rect.size.width += m_outer_margin.vertical();
            // curr_rect.size.height += m_outer_margin.horizontal();

            m_layout_panel->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override

        {
            ds::dims<f32> size{};
            for (auto&& props : m_cell_data | std::views::values)
                size += props.rect.size;
            return size;
        }
    };
}
