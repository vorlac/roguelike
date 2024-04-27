#include "graphics/vg/nanovg.hpp"
#include "scroll_dialog.hpp"
#include "ui/theme.hpp"
#include "ui/widgets/popup.hpp"
#include "utils/logging.hpp"

namespace rl::ui {

    Popup::Popup(Widget* parent, ScrollableDialog* parent_dialog)
        : ScrollableDialog{ parent, "" }
        , m_parent_dialog{ parent_dialog }
    {
    }

    void Popup::set_anchor_pos(const ds::point<f32> anchor_pos)
    {
        m_anchor_pos = anchor_pos;
    }

    ds::point<f32> Popup::anchor_pos() const
    {
        return m_anchor_pos;
    }

    void Popup::set_anchor_offset(const f32 anchor_offset)
    {
        m_anchor_offset = anchor_offset;
    }

    f32 Popup::anchor_offset() const
    {
        return m_anchor_offset;
    }

    void Popup::set_anchor_size(const f32 anchor_size)
    {
        m_anchor_size = anchor_size;
    }

    f32 Popup::anchor_size() const
    {
        return m_anchor_size;
    }

    void Popup::set_side(const Side popup_side)
    {
        m_side = popup_side;
    }

    Side Popup::side() const
    {
        return m_side;
    }

    ScrollableDialog* Popup::parent_dialog()
    {
        return m_parent_dialog;
    }

    const ScrollableDialog* Popup::parent_dialog() const
    {
        return m_parent_dialog;
    }

    void Popup::perform_layout()
    {
        if (m_layout != nullptr || m_children.size() != 1)
            Widget::perform_layout();  // NOLINT(bugprone-parent-virtual-call)
        else {
            const auto first_child{ m_children.front() };
            first_child->set_position(ds::point<f32>::zero());
            first_child->set_size(std::forward<decltype(m_rect.size)>(m_rect.size));
            first_child->perform_layout();
        }

        if (m_side == Side::Left)
            m_anchor_pos.x -= m_rect.size.width;
    }

    void Popup::refresh_relative_placement()
    {
        if (m_parent_dialog == nullptr)
            return;

        m_parent_dialog->refresh_relative_placement();
        m_visible &= m_parent_dialog->visible(true);
        m_rect.pt = m_parent_dialog->position() + m_anchor_pos -
                    ds::point<f32>{ 0.0f, m_anchor_offset };
    }

    void Popup::draw()
    {
        this->refresh_relative_placement();
        if (!m_visible)
            return;

        const auto context{ m_renderer->context() };
        const f32 drop_shadow_size{ m_theme->dialog_drop_shadow_size };
        const f32 corner_radius{ m_theme->dialog_corner_radius };

        m_renderer->scoped_draw([&] {
            nvg::reset_scissor(context);

            // Draw drop shadow behind window
            m_renderer->draw_path(false, [&] {
                const nvg::PaintStyle shadow_paint{ nvg::box_gradient(
                    context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width, m_rect.size.height,
                    corner_radius * 2.0f, drop_shadow_size * 2.0f, m_theme->drop_shadow,
                    m_theme->transparent) };

                nvg::rect(context, m_rect.pt.x - drop_shadow_size, m_rect.pt.y - drop_shadow_size,
                          m_rect.size.width + (2.0f * drop_shadow_size),
                          m_rect.size.height + (2.0f * drop_shadow_size));
                nvg::rounded_rect(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width,
                                  m_rect.size.height, corner_radius);
                nvg::path_winding(context, nvg::Solidity::Hole);
                nvg::fill_paint(context, shadow_paint);
                nvg::fill(context);

                // Draw window
                m_renderer->draw_path(false, [&] {
                    nvg::rounded_rect(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width,
                                      m_rect.size.height, corner_radius);
                    ds::point<f32> base{ ds::point<f32>{ 0.0f, m_anchor_offset } + m_rect.pt };

                    f32 sign{ -1.0f };
                    if (m_side == Side::Left) {
                        base.x += m_rect.size.width;
                        sign = 1.0f;
                    }

                    nvg::move_to(context, base.x + m_anchor_size * sign, base.y);
                    nvg::line_to(context, base.x - (1.0f * sign), base.y - m_anchor_size);
                    nvg::line_to(context, base.x - (1.0f * sign), base.y + m_anchor_size);

                    nvg::fill_color(context, m_theme->dialog_popup_fill);
                    nvg::fill(context);
                });
            });
        });

        Widget::draw();  // NOLINT(bugprone-parent-virtual-call)
    }
}
