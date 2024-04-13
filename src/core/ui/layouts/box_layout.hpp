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
            requires rl::equal_v<VOrientation, Alignment::Horizontal>
        {
            return m_cell_data.size();
        }

        u64 rows() const
            requires rl::equal_v<VOrientation, Alignment::Vertical>
        {
            return m_cell_data.size();
        }

        virtual void apply_layout() override
        {
            // const SizePolicy size_policy{ this->size_policy() };
            ds::point curr_widget_pos{ m_outer_margin.offset() };
            ds::dims cell_size{ m_max_size };

            // cell_size -= m_outer_margin.offset();

            ds::rect layout_rect{ curr_widget_pos, cell_size };

            //   first process and/or recompute the sizes of
            //   each cell that contains a widget in the layout
            for (auto&& [widget, cell] : m_cell_data)
            {
                // compute the widget's layout cell position and size
                ds::rect<f32> widget_bounding_rect{ ds::rect<f32>::zero() };
                ds::rect<f32> widget_margins_rect{ ds::rect<f32>::zero() };

                // this represents the maxiumum amount of space a widget
                // can occupy, including it's inner and outer margins
                widget_margins_rect = ds::rect{
                    curr_widget_pos,
                    cell_size,
                };

                widget_bounding_rect = ds::rect{
                    widget_margins_rect.pt + cell.outer_margin.offset(),
                    widget_margins_rect.size - cell.outer_margin.offset(),
                };

                if (m_alignment == Alignment::Vertical)
                {
                    // widget_bounding_rect.size.height += cell.inner_margin.vertical();
                    curr_widget_pos.y += widget_margins_rect.size.height +
                                         cell.inner_margin.vertical();
                }
                else if (m_alignment == Alignment::Horizontal)
                {
                    // widget_bounding_rect.size.width += cell.inner_margin.horizontal();
                    curr_widget_pos.x += widget_margins_rect.size.width +
                                         cell.inner_margin.horizontal();
                }

                // assign the current child's position and dimensions
                widget->set_rect(widget_bounding_rect);

                // grow the current layout's position and dimensions
                // so that it includes the newly computed child rect
                layout_rect.expand(widget_margins_rect);
            }

            //  increase the total dimensions by the layout widget's
            //  absolute vertical and hotizontal outer margin values
            // layout_rect.size.width += m_outer_margin.right;
            // layout_rect.size.height += m_outer_margin.bottom;

            layout_rect.size += m_inner_margin;

            // layout_rect.size.height += m_inner_margin.bottom;
            //   assign the newly computed rect for the layout widget
            this->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override
        {
            return this->size();
        }
    };
}
