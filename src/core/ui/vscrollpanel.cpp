#include "core/ui/theme.hpp"
#include "core/ui/vscrollpanel.hpp"
#include "ds/color.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"

namespace rl::ui {

    VScrollPanel::VScrollPanel(Widget* parent)
        : Widget{ parent }
        , m_container{ new Widget{ nullptr } }
    {
        m_container->acquire_ref();
        m_container->set_parent(this);
        m_container->set_theme(m_theme);
        m_container->set_visible(true);
    }

    void VScrollPanel::add_child(Widget* child)
    {
        runtime_assert(m_container != nullptr, "vscroll container not set up yet");
        m_container->add_child(child);
    }

    f32 VScrollPanel::scroll() const
    {
        // Return the current scroll amount as a value between 0 and 1. 0 means
        // scrolled to the top and 1 to the bottom.
        return m_scrollbar_pos;
    }

    void VScrollPanel::set_scroll(const f32 scroll)
    {
        // Set the scroll amount to a value between 0 and 1. 0 means scrolled to
        // the top and 1 to the bottom.
        m_scrollbar_pos = scroll;
    }

    Widget* VScrollPanel::container() const
    {
        // Set the scroll amount to a value between 0 and 1. 0 means scrolled to
        // the top and 1 to the bottom.
        return m_container;
    }

    void VScrollPanel::perform_layout()
    {
        scoped_log();

        Widget::perform_layout();
        if (m_container == nullptr)
            return;

        m_cont_prefsize = m_container->preferred_size();
        if (m_cont_prefsize.height > m_size.height)
        {
            m_container->set_position(
                { 0.0f, -m_scrollbar_pos * (m_cont_prefsize.height - m_size.height) });
            m_container->set_size(
                { m_size.width - (Margin + ScrollbarWidth), m_cont_prefsize.height });
        }
        else
        {
            m_container->set_position({ 0.0f, 0.0f });
            m_container->set_size(m_size);
            m_scrollbar_pos = 0;
        }

        m_container->perform_layout();
    }

    ds::dims<f32> VScrollPanel::preferred_size() const
    {
        scoped_log();

        if (m_container == nullptr)
            return ds::dims{ 0.0f, 0.0f };

        return m_container->preferred_size() + ds::dims{ Margin + ScrollbarWidth, 0.0f };
    }

    bool VScrollPanel::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        auto&& mouse_delta{ mouse.pos_delta() };
        if (m_prev_click_location == Component::ScrollBar && m_cont_prefsize.height > m_size.height)
        {
            const float scrollh{ m_size.height *
                                 std::min(1.0f, m_size.height / m_cont_prefsize.height) };
            m_scrollbar_pos = std::max(
                0.0f, std::min(1.0f, m_scrollbar_pos +
                                         mouse_delta.y / (m_size.height - (Margin * 2) - scrollh)));

            m_update_layout = true;
            return true;
        }

        return m_container->on_mouse_drag(mouse, kb);
    }

    bool VScrollPanel::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        {
            LocalTransform transform{ this };
            m_prev_click_location = Component::Body;
            if (m_container->on_mouse_button_pressed(mouse, kb))
                return true;
        }

        const ds::point mouse_pos{ mouse.pos() };
        const ds::point local_mouse_pos{ mouse_pos - LocalTransform::absolute_pos };
        const ds::rect scroll_bar_rect{
            ds::point{ m_pos.x + m_size.width - (ScrollbarWidth + Margin), m_pos.y },
            ds::dims{ ScrollbarWidth, m_size.height }
        };

        const bool lmb_just_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
        if (lmb_just_pressed && m_container != nullptr && m_cont_prefsize.height > m_size.height &&
            scroll_bar_rect.contains(local_mouse_pos))
        {
            const f32 scrollh{ m_size.height *
                               std::min(1.0f, m_size.height / m_cont_prefsize.height) };
            const f32 start{ m_pos.y + Margin + ScrollbarBorder +
                             (m_size.height - ScrollbarWidth - scrollh) * m_scrollbar_pos };

            f32 delta{ 0.0f };
            if (local_mouse_pos.y < start)
                delta = -m_size.height / m_cont_prefsize.height;
            else if (local_mouse_pos.y > start + scrollh)
                delta = m_size.height / m_cont_prefsize.height;

            m_scrollbar_pos = std::max(0.0f, std::min(1.0f, m_scrollbar_pos + delta * 0.98f));
            m_container->set_position(
                { 0.0f, -m_scrollbar_pos * (m_cont_prefsize.height - m_size.height) });

            m_prev_click_location = Component::ScrollBar;
            m_update_layout = true;
            return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        {
            LocalTransform transform{ this };
            if (m_container->on_mouse_button_released(mouse, kb))
                return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        if (m_container == nullptr || m_cont_prefsize.height <= m_size.height)
            return Widget::on_mouse_scroll(mouse, kb);

        const f32 scroll_amount{ mouse.wheel_delta().y * m_size.height * 0.2f };

        m_scrollbar_pos = std::max(
            0.0f, std::min(1.0f, m_scrollbar_pos + (scroll_amount / m_cont_prefsize.height)));

        m_container->set_position({
            0.0f,
            m_scrollbar_pos * (m_cont_prefsize.height - m_size.height),
        });

        {
            LocalTransform transform{ this };
            m_container->on_mouse_move(mouse, kb);
        }

        m_update_layout = true;
        return true;
    }

    void VScrollPanel::draw()
    {
        if (m_container == nullptr)
            return;

        scoped_trace(log_level::trace);

        f32 y_offset{ 0.0f };
        if (m_cont_prefsize.height > m_size.height)
            y_offset = -m_scrollbar_pos * (m_cont_prefsize.height - m_size.height);

        auto&& context{ m_renderer->context() };
        m_container->set_position(ds::point{ 0.0f, y_offset });
        m_cont_prefsize = m_container->preferred_size();
        const f32 scrollh{ m_size.height * math::min(1.0f, m_size.height / m_cont_prefsize.height) };

        if (m_update_layout)
        {
            m_update_layout = false;
            this->perform_layout();
        }

        m_renderer->scoped_draw([&] {
            nvg::translate(context, m_pos.x, m_pos.y);
            nvg::intersect_scissor(context, 0.0f, 0.0f, m_size.width, m_size.height);

            if (m_container->visible())
                m_container->draw();
        });

        if (m_cont_prefsize.height <= m_size.height)
            return;

        m_renderer->draw_path(false, [&] {
            auto scrollbar_bg_rect = ds::rect{
                ds::point{ m_pos.x + m_size.width - (Margin + ScrollbarWidth) + OutlineSize,
                           m_pos.y + Margin + OutlineSize },
                ds::dims{ ScrollbarWidth, m_size.height - Margin * 2.0f },
            };
            auto widget_body_rect = ds::rect{
                ds::point{ m_pos.x + m_size.width - (Margin + ScrollbarWidth), m_pos.y + Margin },
                ds::dims{ ScrollbarWidth, m_size.height - Margin * 2.0f },
            };

            nvg::PaintStyle brush{ m_renderer->create_box_gradient(
                std::move(scrollbar_bg_rect), ScrollBarBackgroundRadius, ShadowBlur,
                ScrollGuideColor, ScrollGuideShadowColor) };

            nvg::rounded_rect(context, std::move(widget_body_rect), ScrollBarBackgroundRadius);
            nvg::fill_paint(context, std::move(brush));
            nvg::fill(context);

            // subpath
            m_renderer->draw_path(false, [&] {
                auto scrollbar_rect = ds::rect{
                    ds::point{ m_pos.x + m_size.width - (Margin + ScrollbarWidth) - OutlineSize,
                               m_pos.y + Margin +
                                   (m_size.height - Margin * 2.0f - scrollh) * m_scrollbar_pos -
                                   OutlineSize },
                    ds::dims{ ScrollbarWidth, scrollh },
                };
                auto scrollbar_border_rect = ds::rect{
                    ds::point{ m_pos.x + m_size.width - (Margin + ScrollbarWidth) + OutlineSize,
                               m_pos.y + Margin + OutlineSize +
                                   (m_size.height - Margin * 2.0f - scrollh) * m_scrollbar_pos },
                    ds::dims{ ScrollbarWidth - Margin / 2.0f, scrollh - Margin / 2.0f },
                };

                nvg::PaintStyle bgbrush{ m_renderer->create_box_gradient(
                    std::move(scrollbar_rect), ScrollBarBackgroundRadius, ShadowBlur,
                    ScrollbarColor, ScrollbarShadowColor) };

                nvg::rounded_rect(context, std::move(scrollbar_border_rect), ScrollBarCornerRadius);
                nvg::fill_paint(context, std::move(bgbrush));
                nvg::fill(context);
            });
        });
    }
}
