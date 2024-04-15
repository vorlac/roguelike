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

            for (auto& widget : m_cell_data | std::views::keys)
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

                if (widget->child_count() == 0)
                {
                    // TODO: figure out how to handle this properly
                    // ds::dims preferred_size{ widget->preferred_size() };
                    // if (m_max_size.width < widget_bounding_rect.size.width)
                    //     widget->set_fixed_height(preferred_size.width);
                    // if (m_max_size.height < widget_bounding_rect.size.height)
                    //     widget->set_fixed_height(preferred_size.height);
                    auto smaller = math::min(m_max_size.width, m_max_size.height);
                    if (widget->has_font_size())
                        widget->set_font_size(smaller);
                }

                widget->set_rect(widget_bounding_rect);
                layout_rect.expand(widget_margins_rect);
            }

            layout_rect.size += m_outer_margin;
            this->set_rect(layout_rect);
        }

        virtual ds::dims<f32> computed_size() const override
        {
            // for (auto& widget : m_cell_data | std::views::keys)
            //     widget->preferred_size();
            return this->size();
        }
    };
}
