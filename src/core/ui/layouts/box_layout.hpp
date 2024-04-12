#pragma once

#include "core/ui/layouts/layout.hpp"
#include "core/ui/widgets/panel.hpp"
#include "ds/dims.hpp"
#include "ds/rect.hpp"
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
            ds::point curr_widget_pos{ ds::point<f32>::zero() };

            const SizePolicy size_policy{ this->size_policy() };
            const ds::dims cell_size{ m_max_size };

            ds::rect layout_rect{ m_rect };
            layout_rect.size -= m_inner_margin;

            //   first process and/or recompute the sizes of
            //   each cell that contains a widget in the layout
            for (auto&& [widget, props] : m_cell_data)
            {
                // compute the widget's layout cell position and size
                ds::rect widget_bounding_rect{ ds::rect<f32>::zero() };
                ds::rect widget_margins_rect{ ds::rect<f32>::zero() };

                widget_margins_rect = ds::rect{
                    layout_rect.pt +
                        ds::point{
                            curr_widget_pos.x,
                            curr_widget_pos.y,
                        },
                    ds::dims{ cell_size },
                };

                widget_bounding_rect = ds::rect{
                    ds::point{
                        widget_margins_rect.pt.x + props.inner_margin.left,
                        widget_margins_rect.pt.y + props.inner_margin.top,
                    },
                    ds::dims{ widget_margins_rect.size - props.outer_margin },
                };

                if (m_alignment == Alignment::Vertical)
                    curr_widget_pos.y += widget_margins_rect.size.height +
                                         props.inner_margin.vertical();
                else if (m_alignment == Alignment::Horizontal)
                    curr_widget_pos.x += widget_margins_rect.size.width +
                                         props.inner_margin.horizontal();

                // assign the current child's position and dimensions
                widget->set_rect(widget_bounding_rect);

                // grow the current layout's position and dimensions
                // so that it includes the newly computed child rect
                layout_rect.expand(widget_margins_rect);
            }

            //  increase the total dimensions by the layout widget's
            //  absolute vertical and hotizontal outer margin values
            layout_rect.size.width += m_outer_margin.right;
            layout_rect.size.height += m_outer_margin.bottom;
            layout_rect.size.width += m_inner_margin.right;
            layout_rect.size.height += m_inner_margin.bottom;
            //   assign the newly computed rect for the layout widget
            this->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override
        {
            return this->size();
        }
    };
}
