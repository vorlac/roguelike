#include "core/ui/theme.hpp"
#include "core/ui/vscrollpanel.hpp"
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

        m_child_preferred_height = m_container->preferred_size().height;
        if (m_child_preferred_height > m_size.height)
        {
            m_container->set_position({
                0.0f,
                -m_scroll * (m_child_preferred_height - m_size.height),
            });

            m_container->set_size({
                m_size.width - (OUTER_MARGIN + SCROLLBAR_WIDTH),
                m_child_preferred_height,
            });
        }
        else
        {
            m_container->set_position({ 0.0f, 0.0f });
            m_container->set_size(m_size);
            m_scroll = 0;
        }

        m_container->perform_layout();
    }

    ds::dims<f32> VScrollPanel::preferred_size() const
    {
        scoped_log();

        if (m_container == nullptr)
            return ds::dims{ 0.0f, 0.0f };

        return m_container->preferred_size() + ds::dims{ (OUTER_MARGIN + SCROLLBAR_WIDTH), 0.0f };
    }

    bool VScrollPanel::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        const auto mouse_delta{ mouse.pos_delta() };
        if (m_prev_click_location == Component::ScrollBar &&
            m_child_preferred_height > m_size.height)
        {
            const float scrollh{ m_size.height *
                                 std::min(1.0f, m_size.height / m_child_preferred_height) };
            m_scroll = std::max(
                0.0f, std::min(1.0f, m_scroll + mouse_delta.y / (m_size.height -
                                                                 (OUTER_MARGIN * 2) - scrollh)));

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
            ds::point{ m_pos.x + m_size.width - (SCROLLBAR_WIDTH + OUTER_MARGIN), m_pos.y },
            ds::dims{ SCROLLBAR_WIDTH, m_size.height }
        };

        const bool lmb_just_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
        if (lmb_just_pressed && m_container != nullptr &&
            m_child_preferred_height > m_size.height && scroll_bar_rect.contains(local_mouse_pos))
        {
            const f32 scrollh{ m_size.height *
                               std::min(1.0f, m_size.height / m_child_preferred_height) };
            const f32 start{ m_pos.y + OUTER_MARGIN + SCROLLBAR_BORDER +
                             (m_size.height - SCROLLBAR_WIDTH - scrollh) * m_scroll };

            f32 delta{ 0.0f };
            if (local_mouse_pos.y < start)
                delta = -m_size.height / m_child_preferred_height;
            else if (local_mouse_pos.y > start + scrollh)
                delta = m_size.height / m_child_preferred_height;

            m_prev_click_location = Component::ScrollBar;
            m_scroll = std::max(0.0f, std::min(1.0f, m_scroll + delta * 0.98f));
            m_container->set_position({
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

        LocalTransform transform{ this };
        if (m_container->on_mouse_button_released(mouse, kb))
        {
            diag_log("Mouse Released");
            return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log();

        if (m_container == nullptr || m_child_preferred_height <= m_size.height)
            return Widget::on_mouse_scroll(mouse, kb);

        const f32 scroll_amount{ mouse.wheel_delta().y * m_size.height * 0.2f };

        m_scroll = std::max(0.0f,
                            std::min(1.0f, m_scroll + (scroll_amount / m_child_preferred_height)));

        m_container->set_position({
            0.0f,
            m_scroll * (m_child_preferred_height - m_size.height),
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

        f32 yoffset{ 0.0f };
        if (m_child_preferred_height > m_size.height)
            yoffset = -m_scroll * (m_child_preferred_height - m_size.height);

        auto&& context{ m_renderer->context() };
        m_container->set_position(ds::point{ 0.0f, yoffset });
        m_child_preferred_height = m_container->preferred_size().height;
        const f32 scrollh{ m_size.height *
                           math::min(1.0f, m_size.height / m_child_preferred_height) };

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

        if (m_child_preferred_height <= m_size.height)
            return;

        ds::rect panel_rect{
            ds::point{
                m_pos.x + m_size.width - (OUTER_MARGIN + SCROLLBAR_WIDTH) + OUTLINE_SIZE,
                m_pos.y + OUTER_MARGIN + OUTLINE_SIZE,
            },
            ds::dims{
                SCROLLBAR_WIDTH,
                m_size.height - (OUTER_MARGIN * 2),
            },
        };

        nvg::PaintStyle brush{
            m_renderer->create_box_gradient(std::forward<ds::rect<f32>>(panel_rect), CORNER_RADIUS,
                                            OUTER_SHADOW_BLUR, ds::color<f32>{ 0, 0, 0, 32 },
                                            ds::color<f32>{ 0, 0, 0, 92 }),
        };

        m_renderer->draw_path(false, [&] {
            nvg::rounded_rect(context,
                              ds::rect{
                                  ds::point{
                                      m_pos.x + m_size.width - (OUTER_MARGIN + SCROLLBAR_WIDTH),
                                      m_pos.y + OUTER_MARGIN,
                                  },
                                  ds::dims{
                                      SCROLLBAR_WIDTH,
                                      m_size.height - (OUTER_MARGIN * 2.0f),
                                  },
                              },
                              CORNER_RADIUS);

            nvg::fill_paint(context, brush);
            nvg::fill(context);

            brush = m_renderer->create_box_gradient(
                ds::rect{
                    ds::point{
                        m_pos.x + m_size.width - (OUTER_MARGIN + SCROLLBAR_WIDTH) - OUTLINE_SIZE,
                        m_pos.y + 4.0f +
                            (m_size.height - (OUTER_MARGIN * 2.0f) - scrollh) * m_scroll -
                            OUTLINE_SIZE,
                    },
                    ds::dims{
                        SCROLLBAR_WIDTH,
                        scrollh,
                    },
                },
                CORNER_RADIUS, OUTER_SHADOW_BLUR, ds::color<f32>{ 220, 220, 220, 100 },
                ds::color<f32>{ 128, 128, 128, 100 });

            // subpath
            m_renderer->draw_path(false, [&] {
                nvg::rounded_rect(
                    context,
                    ds::rect{
                        ds::point{
                            m_pos.x + m_size.width - (OUTER_MARGIN + SCROLLBAR_WIDTH) + OUTLINE_SIZE,
                            m_pos.y + OUTER_MARGIN + OUTLINE_SIZE +
                                (m_size.height - (OUTER_MARGIN * 2.0f) - scrollh) * m_scroll,
                        },
                        ds::dims{
                            SCROLLBAR_WIDTH - (OUTER_MARGIN / 2.0f),
                            scrollh - (OUTER_MARGIN / 2.0f),
                        },
                    },
                    2.0f);

                nvg::fill_paint(context, brush);
                nvg::fill(context);
            });
        });
    }
}
