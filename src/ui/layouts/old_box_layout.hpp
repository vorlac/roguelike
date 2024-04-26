#pragma once

#include "ds/dims.hpp"
#include "ds/rect.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "ui/widget.hpp"
#include "utils/properties.hpp"

namespace rl::nvg {
    struct Context;
}

#include "ui/layouts/layout.hpp"

namespace rl::ui {
    class Widget;

    // template <Orientation VOrientation>
    // class NewBoxLayout final : public Layout
    //{
    // public:
    //     friend class Widget;
    //     constexpr static inline Orientation orientation = VOrientation;

    // public:
    //     constexpr explicit NewBoxLayout() = default;

    //    void add_widget(Widget* widget, CellProperties properties = {})
    //    {
    //        m_cell_data.emplace_back(widget, properties);
    //    }

    //    void add_layout(Layout* layout, CellProperties properties = {})
    //    {
    //        auto widget{ new Widget{ m_owner } };
    //        widget->set_layout(layout);
    //        m_cell_data.emplace_back(widget, properties);
    //    }

    //    u64 columns() const
    //        requires std::same_as<VOrientation, Orientation::Horizontal>
    //    {
    //        return m_cell_data.size();
    //    }

    //    u64 rows() const
    //        requires std::same_as<VOrientation, Orientation::Vertical>
    //    {
    //        return m_cell_data.size();
    //    }

    //    virtual void apply_layout(const u32 depth = 0) override
    //    {
    //        m_layout_rect = {
    //            { 0.0f, 0.0f },
    //            { 0.0f, 0.0f },
    //        };

    //        for (auto&& [widget, props] : m_cell_data)
    //        {
    //            ds::point curr_pos{ 0.0f, 0.0f };

    //            // for any widgets that have children, first apply the layout
    //            // for each recursively so the deepest / leaf nodes in the tree
    //            // of widgets have their layouts appropriately set so their sizes
    //            // are known for the parent / ancestor widgets. this is necessary
    //            // for the higher level widgets that contains others to be able to
    //            // determine their sizes appropriately.
    //            for (const auto child : widget->children())
    //            {
    //                auto child_layout{ child->layout() };
    //                if (child_layout != nullptr)
    //                    child_layout->apply_layout(depth + 1);

    //                // create the widget's rect (position and size) based on it's
    //                // relative position to the parent widget's top-left corner,
    //                // which would be (0,0) in the current local coordinate system
    //            }
    //            ds::rect widget_rect{
    //                ds::point{ props.outer_margin.left, props.outer_margin.top },
    //                widget->fixed_size().merged(widget->preferred_size(/*TODO: add padding*/)),
    //            };

    //            props.rect = ds::rect{
    //                curr_pos,
    //                widget_rect.size +
    //                    ds::vector2{ props.outer_margin.left, props.outer_margin.top } +
    //                    ds::vector2{ props.outer_margin.right, props.outer_margin.bottom },
    //            };

    //            if constexpr (NewBoxLayout::orientation == Orientation::Vertical)
    //            {
    //                // m_layout_rect.size.height += props.rect.size.height;
    //                // if (props.rect.pt.x < m_layout_rect.pt.x)
    //                //     m_layout_rect.pt.x = props.rect.pt.x;
    //                // if (props.rect.size.width > m_layout_rect.size.width)
    //                //     m_layout_rect.size.width = props.rect.size.width;

    //                // assign widget positions from top to bottom
    //                curr_pos.y += props.rect.size.height;
    //                widget->set_rect(std::move(widget_rect));
    //            }
    //            else if constexpr (NewBoxLayout::orientation == Orientation::Horizontal)
    //            {
    //                // m_layout_rect.size.width += props.rect.size.width;
    //                // if (props.rect.pt.y < m_layout_rect.pt.y)
    //                //     m_layout_rect.pt.y = props.rect.pt.y;
    //                // if (props.rect.size.height > m_layout_rect.size.height)
    //                //     m_layout_rect.size.height = props.rect.size.height;

    //                // assign widget positions from left to right
    //                curr_pos.x += props.rect.size.width;
    //                widget->set_rect(std::move(widget_rect));
    //            }

    //            m_layout_rect.expand(props.rect);
    //        }

    //        ds::dims<f32> base_size{ m_owner->fixed_size().merged(
    //            m_owner->preferred_size(/*TODO: add padding*/)) };
    //        ds::rect owner_rect{
    //            ds::point{ m_outer_margin.left, m_outer_margin.top },
    //            ds::dims{ base_size + ds::vector2{ m_outer_margin.left, m_outer_margin.top } +
    //                      ds::vector2{ m_outer_margin.right, m_outer_margin.bottom } },
    //        };

    //        ds::rect test{
    //            ds::point{
    //                0.0f,
    //                0.0f,
    //            },
    //            this->computed_size(),
    //        };

    //        // debug_assert(owner_rect.contains(m_layout_rect) || owner_rect == m_layout_rect,
    //        //                "rect mismatch");

    //        m_owner->set_rect(std::move(owner_rect));
    //    }

    //    // Widget::renderer()->draw_rect_outline(m_layout_rect, depth + 1.0f,
    //    //                                       Colors::List[depth],
    //    //                                       Outline::Inner);

    //    // return Widget::renderer()->draw_rect_outline(m_layout_rect, depth + 1.0f,
    //    //    Colors::White, Outline::Inner);

    //    virtual ds::dims<f32> computed_size() const override
    //    {
    //        ds::dims<f32> size{};
    //        for (const auto& props : m_cell_data | std::views::values)
    //            size += props.rect.size;
    //        return size;
    //    }
    //};

    // Simple layout that supports horizontal and vertical orientation.
    // Aside form defining the Layout interface for sizing and
    // performing the Layout, a BoxLayout only handles basic orientation,
    // margins and spacing.
    class OldBoxLayout final : public OldLayout
    {
    public:
        explicit OldBoxLayout(Alignment orientation,
                              Placement_OldAlignment alignment = Placement_OldAlignment::Center,
                              f32 margin = 0.0f, f32 spacing = 0.0f);

        f32 margin() const;
        f32 spacing() const;
        Placement_OldAlignment alignment() const;
        Alignment orientation() const;

        void set_margin(f32 margin);
        void set_spacing(f32 spacing);
        void set_orientation(Alignment orientation);
        void set_alignment(Placement_OldAlignment alignment);

    public:
        virtual void apply_layout(nvg::Context* nvg_context, const Widget* widget) const override;
        virtual ds::dims<f32> computed_size(nvg::Context* nvg_context,
                                            const Widget* widget) const override;

    protected:
        f32 m_margin{ 0.0f };
        f32 m_spacing{ 0.0f };
        Alignment m_orientation{};
        Placement_OldAlignment m_alignment{};
    };
}
