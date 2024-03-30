#pragma once

#include "core/ui/widgets/panel.hpp"
#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/properties.hpp"

namespace rl::nvg {
    struct Context;
}

#include "core/ui/layouts/layout.hpp"

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
            // recursively apply any nested layouts first
            if (m_layout_panel != nullptr)
            {
                auto sub_layout{ m_layout_panel->layout() };
                if (sub_layout != nullptr)
                    sub_layout->apply_layout(depth + 1);
            }

            ds::point curr_pos{ 0.0f, 0.0f };
            ds::rect curr_rect{ 0.0f, 0.0f, 0.0f, 0.0f };
            // first process and/or recompute the sizes of
            // each cell that contains a widget in the layout
            for (auto&& [widget, props] : m_cell_data)
            {
                // for each layout widget, recursively update the sizes of each leaf
                // node widget in the tree below the widget being processed
                for (Widget* child : widget->children())
                {
                    auto child_layout{ child->layout() };
                    if (child_layout != nullptr)
                        child_layout->apply_layout(depth + 1);

                    // once we pass beyond the point where all lower level widget sizes
                    // have updated and/or recalculated, start calculating its placement
                    const ds::dims child_fixd_size{ child->fixed_size() };
                    const ds::dims child_pref_size{ child->preferred_size() };
                    const ds::dims child_actual_size{ child_fixd_size.merged(child_pref_size) };
                    child->set_size(child_actual_size);
                }

                // create the widget's rect (position and size) based on it's
                // relative position to the parent widget's top-left corner,
                // which would be (0,0) in the current local coordinate system
                const ds::dims widget_pref_size{ widget->preferred_size() };
                const ds::dims widget_fixed_size{ widget->fixed_size() };
                // this should be the widget size minus any external margins or padding
                const ds::dims widget_actual_size{ widget_fixed_size.merged(widget_pref_size) };
                // compute the widget's layout cell position and size (including external margins)
                const ds::rect widget_bounding_rect{
                    curr_pos +
                        ds::vector2{
                            props.outer_margin.left,
                            props.outer_margin.top,
                        },
                    widget_actual_size +
                        ds::vector2{
                            props.outer_margin.right,
                            props.outer_margin.bottom,
                        },
                };

                widget->set_rect(widget_bounding_rect);

                // assign positions from top to bottom
                if constexpr (orientation == Arrangement::Vertical)
                    curr_pos.y += props.rect.size.height;
                // assign positions from left to right
                if constexpr (orientation == Arrangement::Horizontal)
                    curr_pos.x += props.rect.size.width;

                curr_rect.expand(widget_bounding_rect);
            }

            curr_rect.size.width += m_outer_margin.vertical();
            curr_rect.size.height += m_outer_margin.horizontal();

            m_layout_panel->set_rect(curr_rect);
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
