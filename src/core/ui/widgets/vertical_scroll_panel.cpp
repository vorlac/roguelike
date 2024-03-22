#include "core/ui/theme.hpp"
#include "ds/color.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"
#include "vertical_scroll_panel.hpp"

namespace rl::ui {

    VerticalScrollPanel::VerticalScrollPanel(Widget* parent)
        : Widget{ parent }
    {
        m_container = new ScrollableContainer{ nullptr };
        Widget::add_child(0, m_container);
    }

    f32 VerticalScrollPanel::scroll() const
    {
        // Return the current scroll amount as a value between 0 and 1.
        // 0 means scrolled to the top and 1 to the bottom.
        return m_scrollbar_pos;
    }

    void VerticalScrollPanel::set_scroll(const f32 scroll)
    {
        // Set the scroll amount to a value between 0 and 1.
        // 0 means scrolled to the top and 1 to the bottom.
        m_scrollbar_pos = scroll;
    }

    Widget* VerticalScrollPanel::container() const
    {
        // Set the scroll amount to a value between 0 and 1.
        // 0 means scrolled to the top and 1 to the bottom.
        return m_container;
    }

    void VerticalScrollPanel::perform_layout()
    {
        scoped_log();

        Widget::perform_layout();
        if (m_container == nullptr)
            return;

        m_cont_prefsize = m_container->preferred_size();
        if (m_cont_prefsize.height > m_rect.size.height)
        {
            m_scroll_bar_rect = ds::rect{
                ds::point{ m_rect.pt.x + m_rect.size.width - (ScrollbarWidth + Margin),
                           m_rect.pt.y },
                ds::dims{ ScrollbarWidth, m_rect.size.height },
            };

            m_container->set_rect({
                ds::point{
                    0.0f,
                    -m_scrollbar_pos * (m_cont_prefsize.height - m_rect.size.height),
                },
                ds::dims{
                    m_rect.size.width - (Margin + ScrollbarWidth),
                    m_cont_prefsize.height,
                },
            });
        }
        else
        {
            m_scrollbar_pos = 0.0f;
            m_container->set_rect({
                { 0.0f, 0.0f },
                std::forward<decltype(m_rect.size)>(m_rect.size),
            });
        }

        m_container->perform_layout();
    }

    ds::dims<f32> VerticalScrollPanel::preferred_size() const
    {
        scoped_log();

        if (m_container == nullptr)
            return { 0.0f, 0.0f };

        return m_container->preferred_size() + ds::dims{ Margin + ScrollbarWidth, 0.0f };
    }

    Widget* VerticalScrollPanel::find_widget(const ds::point<f32>& pt)
    {
        scoped_trace(log_level::debug);
        if (m_scroll_bar_rect.contains(pt))
            return this;

        return m_container->find_widget(pt - m_rect.pt);
    }

    bool VerticalScrollPanel::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        const auto mouse_delta{ mouse.pos_delta() };
        if (m_prev_click_location == Component::ScrollBar &&
            m_cont_prefsize.height > m_rect.size.height)
        {
            const float scrollbar_height{
                m_rect.size.height * std::min(1.0f, m_rect.size.height / m_cont_prefsize.height)
            };
            m_scrollbar_pos = std::max(
                0.0f, std::min(1.0f, m_scrollbar_pos +
                                         mouse_delta.y / (m_rect.size.height - (Margin * 2) -
                                                          scrollbar_height)));
            m_update_layout = true;
            return true;
        }

        LocalTransform transform{ this };
        return Widget::on_mouse_drag(mouse, kb);
    }

    bool VerticalScrollPanel::draw_mouse_intersection(const ds::point<f32>& pt)
    {
        scoped_logger(log_level::trace, "pos={}", pt);

        if (this->contains(pt))
        {
            const ds::rect widget_rect{ m_rect.pt, m_rect.size };
            m_renderer->draw_rect_outline(widget_rect, 2.0f, rl::Colors::Green, Outline::Inner);
        }

        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ pt - m_rect.pt };
        return m_container->draw_mouse_intersection(local_mouse_pos);
    }

    bool VerticalScrollPanel::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        m_prev_click_location = Component::None;
        const ds::point mouse_pos{ mouse.pos() };
        const ds::point local_mouse_pos{ mouse_pos - LocalTransform::absolute_pos };

        const bool lmb_just_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
        if (lmb_just_pressed && m_container != nullptr &&
            m_cont_prefsize.height > m_rect.size.height &&
            m_scroll_bar_rect.contains(local_mouse_pos))
        {
            m_prev_click_location = Component::ScrollBar;
            const f32 scrollbar_height{
                m_rect.size.height * std::min(1.0f, m_rect.size.height / m_cont_prefsize.height)
            };
            const f32 start{ m_rect.pt.y + Margin + ScrollbarBorder +
                             (m_rect.size.height - ScrollbarWidth - scrollbar_height) *
                                 m_scrollbar_pos };

            f32 delta{ 0.0f };
            if (local_mouse_pos.y < start)
                delta = -m_rect.size.height / m_cont_prefsize.height;
            else if (local_mouse_pos.y > start + scrollbar_height)
                delta = m_rect.size.height / m_cont_prefsize.height;

            m_scrollbar_pos = std::max(0.0f, std::min(1.0f, m_scrollbar_pos + delta));
            m_container->set_position(
                { 0.0f, -m_scrollbar_pos * (m_cont_prefsize.height - m_rect.size.height) });

            m_update_layout = true;
            return true;
        }

        LocalTransform transform{ this };
        return m_container->on_mouse_button_pressed(mouse, kb);
    }

    bool VerticalScrollPanel::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::trace, "pos={}", mouse.pos());

        LocalTransform transform{ this };
        return m_container->on_mouse_button_released(mouse, kb);
    }

    bool VerticalScrollPanel::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::trace, "pos={}", mouse.pos());

        LocalTransform transform{ this };
        return m_container->on_mouse_move(mouse, kb);
    }

    bool VerticalScrollPanel::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        if (m_container == nullptr || m_cont_prefsize.height <= m_rect.size.height)
            return Widget::on_mouse_scroll(mouse, kb);

        const f32 scroll_amount{ mouse.wheel_delta().y * m_rect.size.height * 0.2f };

        m_scrollbar_pos = std::max(
            0.0f, std::min(1.0f, m_scrollbar_pos + (scroll_amount / m_cont_prefsize.height)));

        m_container->set_position({
            0.0f,
            m_scrollbar_pos * (m_cont_prefsize.height - m_rect.size.height),
        });

        {
            LocalTransform transform{ this };
            m_container->on_mouse_move(mouse, kb);
        }

        m_update_layout = true;
        return true;
    }

    void VerticalScrollPanel::draw()
    {
        if (m_container == nullptr)
            return;

        scoped_trace(log_level::trace);

        f32 y_offset{ 0.0f };
        if (m_cont_prefsize.height > m_rect.size.height)
            y_offset = -m_scrollbar_pos * (m_cont_prefsize.height - m_rect.size.height);

        const auto context{ m_renderer->context() };
        m_container->set_position({ 0.0f, y_offset });
        m_cont_prefsize = m_container->preferred_size();
        const f32 scrollbar_height{ m_rect.size.height *
                                    math::min(1.0f, m_rect.size.height / m_cont_prefsize.height) };

        if (m_update_layout)
        {
            m_update_layout = false;
            this->perform_layout();
        }

        m_renderer->scoped_draw([&] {
            LocalTransform transform{ this };
            nvg::intersect_scissor(context, 0.0f, 0.0f, m_rect.size.width, m_rect.size.height);
            if (m_container->visible())
                m_container->draw();
        });

        if (m_cont_prefsize.height <= m_rect.size.height)
            return;

        m_renderer->draw_path(false, [&] {
            ds::rect scrollbar_bg_rect{
                ds::point{ m_rect.pt.x + m_rect.size.width - (Margin + ScrollbarWidth) + OutlineSize,
                           m_rect.pt.y + Margin + OutlineSize },
                ds::dims{ ScrollbarWidth, m_rect.size.height - Margin * 2.0f },
            };
            ds::rect widget_body_rect{
                ds::point{ m_rect.pt.x + m_rect.size.width - (Margin + ScrollbarWidth),
                           m_rect.pt.y + Margin },
                ds::dims{ ScrollbarWidth, m_rect.size.height - Margin * 2.0f },
            };

            nvg::PaintStyle brush{ m_renderer->create_rect_gradient_paint_style(
                std::move(scrollbar_bg_rect), ScrollBarBackgroundRadius, ShadowBlur,
                ScrollGuideColor, ScrollGuideShadowColor) };

            nvg::rounded_rect(context, std::move(widget_body_rect), ScrollBarBackgroundRadius);
            nvg::fill_paint(context, std::move(brush));
            nvg::fill(context);

            // subpath
            m_renderer->draw_path(false, [&] {
                auto scrollbar_rect = ds::rect{
                    ds::point{ m_rect.pt.x + m_rect.size.width - (Margin + ScrollbarWidth) -
                                   OutlineSize,
                               m_rect.pt.y + Margin +
                                   (m_rect.size.height - Margin * 2.0f - scrollbar_height) *
                                       m_scrollbar_pos -
                                   OutlineSize },
                    ds::dims{ ScrollbarWidth, scrollbar_height },
                };
                auto scrollbar_border_rect = ds::rect{
                    ds::point{ m_rect.pt.x + m_rect.size.width - (Margin + ScrollbarWidth) +
                                   OutlineSize,
                               m_rect.pt.y + Margin + OutlineSize +
                                   (m_rect.size.height - Margin * 2.0f - scrollbar_height) *
                                       m_scrollbar_pos },
                    ds::dims{ ScrollbarWidth - Margin / 2.0f, scrollbar_height - Margin / 2.0f },
                };

                nvg::PaintStyle bgbrush{ m_renderer->create_rect_gradient_paint_style(
                    std::move(scrollbar_rect), ScrollBarBackgroundRadius, ShadowBlur,
                    ScrollbarColor, ScrollbarShadowColor) };

                nvg::rounded_rect(context, std::move(scrollbar_border_rect), ScrollBarCornerRadius);
                nvg::fill_paint(context, std::move(bgbrush));
                nvg::fill(context);
            });
        });
    }
}
