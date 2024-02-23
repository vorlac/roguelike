#include "core/ui/theme.hpp"
#include "core/ui/vscrollpanel.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"

namespace rl::ui {

    VScrollPanel::VScrollPanel(Widget* parent)
        : Widget{ parent }
    {
    }

    f32 VScrollPanel::scroll() const
    {
        // Return the current scroll amount as a value between 0 and 1. 0 means
        // scrolled to the top and 1 to the bottom.
        return m_scroll;
    }

    void VScrollPanel::set_scroll(const f32 scroll)
    {
        // Set the scroll amount to a value between 0 and 1. 0 means scrolled to
        // the top and 1 to the bottom.
        m_scroll = scroll;
    }

    void VScrollPanel::perform_layout()
    {
        scoped_log();

        Widget::perform_layout();
        if (m_children.empty())
            return;

        runtime_assert(m_children.size() == 1, "vertical scroll panel should only have 1 child");

        Widget* child{ m_children.front() };
        m_child_preferred_height = child->preferred_size().height;

        if (m_child_preferred_height > m_size.height)
        {
            child->set_position({
                0.0f,
                -m_scroll * (m_child_preferred_height - m_size.height),
            });

            child->set_size({
                m_size.width - 12.0f,
                m_child_preferred_height,
            });
        }
        else
        {
            child->set_position({ 0.0f, 0.0f });
            child->set_size(m_size);
            m_scroll = 0;
        }

        child->perform_layout();
    }

    ds::dims<f32> VScrollPanel::preferred_size() const
    {
        scoped_log();

        if (m_children.empty())
            return ds::dims{ 0.0f, 0.0f };

        return m_children.front()->preferred_size() + ds::dims{ 12.0f, 0.0f };
    }

    bool VScrollPanel::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        if (m_children.empty() || m_child_preferred_height <= m_size.height)
            return Widget::on_mouse_drag(mouse, kb);

        auto&& mouse_delta{ mouse.pos_delta() };
        const f32 scrollh{ this->height() *
                           std::min(1.0f, this->height() / m_child_preferred_height) };

        m_scroll = std::max(
            0.0f, std::min(1.0f, m_scroll + (mouse_delta.y / (m_size.height - 8.0f - scrollh))));

        m_update_layout = true;
        return true;
    }

    bool VScrollPanel::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        scoped_log();
        const auto&& mouse_pos{ mouse.pos() };
        const bool lmb_just_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };

        if (lmb_just_pressed && !m_children.empty() && m_child_preferred_height > m_size.height &&
            mouse_pos.x > m_pos.x + m_size.width - 13.0f &&
            mouse_pos.x < m_pos.x + m_size.width - 4.0f)
        {
            const f32 scrollh{ this->height() *
                               std::min(1.0f, this->height() / m_child_preferred_height) };

            const f32 start{ m_pos.y + 4.0f + 1.0f + (m_size.height - 8.0f - scrollh) * m_scroll };

            f32 delta{ 0.0f };
            if (mouse_pos.y < start)
                delta = -m_size.height / m_child_preferred_height;
            else if (mouse_pos.y > start + scrollh)
                delta = m_size.height / m_child_preferred_height;

            m_scroll = std::max(0.0f, std::min(1.0f, m_scroll + delta * 0.98f));
            m_children.front()->set_position({
                0.0f,
                -m_scroll * (m_child_preferred_height - m_size.height),
            });

            m_update_layout = true;
            return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();
        if (Widget::on_mouse_button_released(mouse, kb))
            return true;

        if (Widget::on_mouse_button_released(mouse, kb))
        {
            diag_log("Mouse Released");
            return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        if (m_children.empty() || m_child_preferred_height <= m_size.height)
            return Widget::on_mouse_scroll(mouse, kb);

        Widget* child{ m_children.front() };
        const f32 scroll_amount{ mouse.wheel_delta().y * m_size.height * 0.2f };

        m_scroll = std::max(0.0f,
                            std::min(1.0f, m_scroll + (scroll_amount / m_child_preferred_height)));

        child->set_position({
            0.0f,
            m_scroll * (m_child_preferred_height - m_size.height),
        });

        m_update_layout = true;
        child->on_mouse_move(mouse, kb);

        return true;
    }

    void VScrollPanel::draw()
    {
        if (m_children.empty())
            return;

        scoped_trace(log_level::trace);

        Widget* child{ m_children[0] };

        f32 yoffset{ 0.0f };
        if (m_child_preferred_height > m_size.height)
            yoffset = -m_scroll * (m_child_preferred_height - m_size.height);

        auto&& context{ m_renderer->context() };
        child->set_position(ds::point{ 0.0f, yoffset });
        m_child_preferred_height = child->preferred_size().height;
        const f32 scrollh{ this->height() *
                           math::min(1.0f, this->height() / m_child_preferred_height) };

        if (m_update_layout)
        {
            m_update_layout = false;
            child->perform_layout();
        }

        m_renderer->scoped_draw([&] {
            nvg::translate(context, m_pos.x, m_pos.y);
            nvg::intersect_scissor(context, 0.0f, 0.0f, m_size.width, m_size.height);

            if (child->visible())
                child->draw();
        });

        if (m_child_preferred_height <= m_size.height)
            return;

        constexpr static f32 OUTLINE_SIZE{ 1.0f };
        ds::rect panel_rect{
            ds::point{
                m_pos.x + m_size.width - 12.0f + OUTLINE_SIZE,
                m_pos.y + 4.0f + OUTLINE_SIZE,
            },
            ds::dims{
                8.0f,
                m_size.height - 8.0f,
            },
        };

        nvg::PaintStyle paint{ m_renderer->create_box_gradient(
            std::forward<ds::rect<f32>>(panel_rect), 3.0f, 4.0f, ds::color<f32>{ 0, 0, 0, 32 },
            ds::color<f32>{ 0, 0, 0, 92 }) };

        m_renderer->draw_path(false, [&] {
            nvg::rounded_rect(context, m_pos.x + m_size.width - 12.0f, m_pos.y + 4.0f, 8.0f,
                              m_size.height - 8.0f, 3.0f);
            nvg::fill_paint(context, paint);
            nvg::fill(context);

            paint = m_renderer->create_box_gradient(
                ds::rect{
                    ds::point{
                        m_pos.x + m_size.width - 12.0f - OUTLINE_SIZE,
                        m_pos.y + 4.0f + (m_size.height - 8.0f - scrollh) * m_scroll - OUTLINE_SIZE,
                    },
                    ds::dims{
                        8.0f,
                        scrollh,
                    },
                },
                3.0f, 4.0f, ds::color<f32>{ 220, 220, 220, 100 },
                ds::color<f32>{ 128, 128, 128, 100 });
            // paint = nvg::box_gradient(
            //     context, m_pos.x + m_size.width - 12.0f - OUTLINE_SIZE,
            //     m_pos.y + 4.0f + (m_size.height - 8.0f - scrollh) * m_scroll -
            //     OUTLINE_SIZE, 8.0f, scrollh, 3.0f, 4.0f, ds::color<f32>{ 220, 220, 220,
            //     100 }, ds::color<f32>{ 128, 128, 128, 100 });

            // subpath
            m_renderer->draw_path(false, [&] {
                nvg::rounded_rect(
                    context, m_pos.x + m_size.width - 12.0f + OUTLINE_SIZE,
                    m_pos.y + 4.0f + OUTLINE_SIZE + (m_size.height - 8.0f - scrollh) * m_scroll,
                    8.0f - 2.0f, scrollh - 2.0f, 2.0f);

                nvg::fill_paint(context, paint);
                nvg::fill(context);
            });
        });
    }
}
