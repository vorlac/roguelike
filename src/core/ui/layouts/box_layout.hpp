#pragma once

#include "core/ui/layouts/layout.hpp"
#include "core/ui/widgets/panel.hpp"
#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/nanovg_state.hpp"
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

        virtual void apply_layout() override
        {
            ds::rect<f32> computed_rect{ ds::rect<f32>::zero() };
            ds::point<f32> curr_widget_pos{ m_inner_margin.offset() };

            for (auto&& widget : m_cell_data | std::views::keys)
            {
                ds::rect<f32> widget_rect{ ds::rect<f32>::zero() };
                Layout* widget_layout{ widget->layout() };
                if (widget_layout == nullptr)
                {
                    widget_rect.size = widget->preferred_size();
                    widget_rect.pt += curr_widget_pos + m_outer_margin.offset();
                }
                else
                {
                    widget_layout->apply_layout();
                    widget_rect = widget_layout->rect();
                    widget_rect.pt += curr_widget_pos;
                    widget_layout->set_rect(widget_rect);
                }

                widget->set_rect(widget_rect);
                computed_rect.expand(widget_rect);

                switch (this->alignment())
                {
                    case Alignment::Horizontal:
                        curr_widget_pos.x += widget_rect.size.width + m_spacing;
                        break;
                    case Alignment::Vertical:
                        curr_widget_pos.y += widget_rect.size.height + m_spacing;
                        break;
                    default:
                        assert_msg("Unsupported layout alignment: {}", this->alignment());
                        break;
                }
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
            for (auto& widget : m_cell_data | std::views::keys)
            {
                Layout* widget_layout{ widget->layout() };
                ds::dims<f32> widget_computed_size{
                    widget_layout != nullptr ? widget_layout->computed_size()
                                             : widget->preferred_size(),
                };

                switch (this->alignment())
                {
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
                        assert_msg("Unsupported layout alignment: {}", this->alignment());
                        break;
                }

                computed_size += m_inner_margin;
                computed_size += m_outer_margin;
            }

            return computed_size;
        }
    };
}
