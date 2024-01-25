#include "core/ui/popup.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl::ui {

    Popup::Popup(Widget* parent, Dialog* parent_dialog)
        : Dialog{ parent, "" }
        , m_parent_dialog{ parent_dialog }
        , m_anchor_pos{ 0.0f, 0.0f }
        , m_anchor_offset{ 30.0f }
        , m_anchor_size{ 15.0f }
        , m_side{ Popup::Side::Right }
    {
    }

    void Popup::set_anchor_pos(const ds::point<f32>& anchor_pos)
    {
        m_anchor_pos = anchor_pos;
    }

    const ds::point<f32> Popup::anchor_pos() const
    {
        return m_anchor_pos;
    }

    void Popup::set_anchor_offset(f32 anchor_offset)
    {
        m_anchor_offset = anchor_offset;
    }

    f32 Popup::anchor_offset() const
    {
        return m_anchor_offset;
    }

    void Popup::set_anchor_size(f32 anchor_size)
    {
        m_anchor_size = anchor_size;
    }

    f32 Popup::anchor_size() const
    {
        return m_anchor_size;
    }

    void Popup::set_side(Popup::Side popup_side)
    {
        m_side = popup_side;
    }

    Popup::Side Popup::side() const
    {
        return m_side;
    }

    Dialog* Popup::parent_window()
    {
        return m_parent_dialog;
    }

    const Dialog* Popup::parent_window() const
    {
        return m_parent_dialog;
    }

    void Popup::perform_layout()
    {
        if (m_layout != nullptr || m_children.size() != 1)
            Widget::perform_layout();
        else
        {
            m_children[0]->set_position({ 0.0f, 0.0f });
            m_children[0]->set_size(m_size);
            m_children[0]->perform_layout();
        }

        if (m_side == Side::Left)
            m_anchor_pos.x -= m_size.width;
    }

    void Popup::refresh_relative_placement()
    {
        if (m_parent_dialog == nullptr)
            return;

        m_parent_dialog->refresh_relative_placement();
        m_visible &= m_parent_dialog->visible_recursive();
        m_pos = m_parent_dialog->position() + m_anchor_pos -
                ds::point<f32>{ 0.0f, m_anchor_offset };
    }

    void Popup::draw()
    {
        this->refresh_relative_placement();
        if (!m_visible)
            return;

        auto&& context{ m_renderer->context() };
        const f32 drop_shadow_size{ m_theme->dialog_drop_shadow_size };
        const f32 corner_radius{ m_theme->dialog_corner_radius };

        nvg::Save(context);
        nvg::ResetScissor(context);

        // Draw a drop shadow
        nvg::NVGpaint shadow_paint = nvg::BoxGradient(
            context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius * 2.0f,
            drop_shadow_size * 2.0f, m_theme->drop_shadow.nvg(), m_theme->transparent.nvg());

        nvg::BeginPath(context);
        nvg::Rect(context, m_pos.x - drop_shadow_size, m_pos.y - drop_shadow_size,
                  m_size.width + (2.0f * drop_shadow_size),
                  m_size.height + (2.0f * drop_shadow_size));
        nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        nvg::PathWinding(context, nvg::NVG_HOLE);
        nvg::FillPaint(context, shadow_paint);
        nvg::Fill(context);

        // Draw window
        nvg::BeginPath(context);
        nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        ds::point<f32> base{ m_pos + ds::point{ 0.0f, m_anchor_offset } };

        f32 sign = -1.0f;
        if (m_side == Side::Left)
        {
            base.x += m_size.width;
            sign = 1.0f;
        }

        nvg::MoveTo(context, base.x + m_anchor_size * sign, base.y);
        nvg::LineTo(context, base.x - (1.0f * sign), base.y - m_anchor_size);
        nvg::LineTo(context, base.x - (1.0f * sign), base.y + m_anchor_size);

        nvg::FillColor(context, m_theme->dialog_popup_fill.nvg());
        nvg::Fill(context);
        nvg::Restore(context);

        Widget::draw();
    }
}
