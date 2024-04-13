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
            ds::dims<f32> cell_size{ m_max_size };
            ds::point<f32> curr_widget_pos{ ds::point<f32>::zero() };
            ds::rect<f32> layout_rect{ curr_widget_pos + m_outer_margin.offset(), cell_size };

            for (auto&& [widget, cell] : m_cell_data)
            {
                ds::rect<f32> widget_margins_rect{
                    curr_widget_pos + m_outer_margin.offset(),
                    cell_size + m_outer_margin,
                };

                ds::rect<f32> widget_bounding_rect{
                    widget_margins_rect.pt + m_inner_margin.offset(),
                    widget_margins_rect.size - m_inner_margin,
                };

                if (m_alignment == Alignment::Vertical)
                    curr_widget_pos.y += widget_margins_rect.size.height;
                else if (m_alignment == Alignment::Horizontal)
                    curr_widget_pos.x += widget_margins_rect.size.width;

                widget->set_rect(widget_bounding_rect);
                layout_rect.expand(widget_margins_rect);
            }

            layout_rect.size += m_outer_margin;
            this->set_rect(std::move(layout_rect));
        }

        virtual ds::dims<f32> computed_size() const override
        {
            return this->size();
        }
    };
}
