#include "core/ui/popup.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/logging.hpp"

namespace rl::ui {

    Popup::Popup(Widget* parent, Dialog* parent_dialog)
        : Dialog{ parent, "" }
        , m_parent_dialog{ parent_dialog }
    {
        scoped_trace(log_level::debug);
    }

    void Popup::set_anchor_pos(ds::point<f32> anchor_pos)
    {
        scoped_logger(log_level::debug, "anchro_pos={}", anchor_pos);
        m_anchor_pos = anchor_pos;
    }

    ds::point<f32> Popup::anchor_pos() const
    {
        scoped_logger(log_level::debug, "m_anchor_pos={}", m_anchor_pos);
        return m_anchor_pos;
    }

    void Popup::set_anchor_offset(f32 anchor_offset)
    {
        scoped_logger(log_level::debug, "anchor_offset={}", anchor_offset);
        m_anchor_offset = anchor_offset;
    }

    f32 Popup::anchor_offset() const
    {
        scoped_logger(log_level::debug, "m_anchor_offset={}", m_anchor_offset);
        return m_anchor_offset;
    }

    void Popup::set_anchor_size(f32 anchor_size)
    {
        scoped_logger(log_level::debug, "anchor_size={}", anchor_size);
        m_anchor_size = anchor_size;
    }

    f32 Popup::anchor_size() const
    {
        scoped_logger(log_level::debug, "m_anchor_size={}", m_anchor_size);
        return m_anchor_size;
    }

    void Popup::set_side(Popup::Side popup_side)
    {
        scoped_logger(log_level::debug, "side={}", popup_side);
        m_side = popup_side;
    }

    Popup::Side Popup::side() const
    {
        scoped_logger(log_level::debug, "side={}", m_side);
        return m_side;
    }

    Dialog* Popup::parent_window()
    {
        scoped_trace(log_level::debug);
        return m_parent_dialog;
    }

    const Dialog* Popup::parent_window() const
    {
        scoped_trace(log_level::debug);
        return m_parent_dialog;
    }

    void Popup::perform_layout()
    {
        scoped_trace(log_level::debug);
        if (m_layout != nullptr || m_children.size() != 1)
            Widget::perform_layout();
        else
        {
            auto&& first_child{ m_children.front() };
            first_child->set_position({ 0.0f, 0.0f });
            first_child->set_size(m_size);
            first_child->perform_layout();
        }

        if (m_side == Side::Left)
            m_anchor_pos.x -= m_size.width;
    }

    void Popup::refresh_relative_placement()
    {
        scoped_trace(log_level::debug);
        if (m_parent_dialog == nullptr)
            return;

        m_parent_dialog->refresh_relative_placement();
        m_visible &= m_parent_dialog->visible_recursive();
        m_pos = m_parent_dialog->position() + m_anchor_pos -
                ds::point<f32>{ 0.0f, m_anchor_offset };
    }

    void Popup::draw()
    {
        scoped_trace(log_level::debug);
        this->refresh_relative_placement();
        if (!m_visible)
            return;

        auto&& context{ m_renderer->context() };
        const f32 drop_shadow_size{ m_theme->dialog_drop_shadow_size };
        const f32 corner_radius{ m_theme->dialog_corner_radius };

        nvg::save(context);
        nvg::reset_scissor(context);

        // Draw a drop shadow
        nvg::PaintStyle shadow_paint = nvg::box_gradient(
            context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius * 2.0f,
            drop_shadow_size * 2.0f, m_theme->drop_shadow, m_theme->transparent);

        nvg::begin_path(context);
        nvg::rect(context, m_pos.x - drop_shadow_size, m_pos.y - drop_shadow_size,
                  m_size.width + (2.0f * drop_shadow_size),
                  m_size.height + (2.0f * drop_shadow_size));
        nvg::rounded_rect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        nvg::path_winding(context, nvg::NVGHole);
        nvg::fill_paint(context, shadow_paint);
        nvg::fill(context);

        // Draw window
        nvg::begin_path(context);
        nvg::rounded_rect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        ds::point<f32> base{ ds::point{ 0.0f, m_anchor_offset } + m_pos };

        f32 sign = -1.0f;
        if (m_side == Side::Left)
        {
            base.x += m_size.width;
            sign = 1.0f;
        }

        nvg::move_to(context, base.x + m_anchor_size * sign, base.y);
        nvg::line_to(context, base.x - (1.0f * sign), base.y - m_anchor_size);
        nvg::line_to(context, base.x - (1.0f * sign), base.y + m_anchor_size);

        nvg::fill_color(context, m_theme->dialog_popup_fill);
        nvg::fill(context);
        nvg::restore(context);

        Widget::draw();
    }
}
