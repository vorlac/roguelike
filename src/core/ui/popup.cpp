#include "core/ui/popup.hpp"
#include "core/ui/theme.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl::ui {

    Popup::Popup(ui::Widget* parent, ui::Dialog* parent_dialog)
        : ui::Dialog{ parent, "" }
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

    ui::Dialog* Popup::parent_window()
    {
        return m_parent_dialog;
    }

    const ui::Dialog* Popup::parent_window() const
    {
        return m_parent_dialog;
    }

    void Popup::perform_layout(nvg::NVGcontext* ctx)
    {
        if (m_layout != nullptr || m_children.size() != 1)
            ui::Widget::perform_layout(ctx);
        else
        {
            m_children[0]->set_position({ 0.0f, 0.0f });
            m_children[0]->set_size(m_size);
            m_children[0]->perform_layout(ctx);
        }

        if (m_side == Side::Left)
            m_anchor_pos.x -= size().width;
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

    void Popup::draw(nvg::NVGcontext* ctx)
    {
        this->refresh_relative_placement();
        if (!m_visible)
            return;

        f32 ds{ m_theme->m_window_drop_shadow_size };
        f32 cr{ m_theme->m_window_corner_radius };

        nvg::Save(ctx);
        nvg::ResetScissor(ctx);

        // Draw a drop shadow
        nvg::NVGpaint shadow_paint = nvg::BoxGradient(
            ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr * 2.0f, ds * 2.0f,
            m_theme->m_drop_shadow, m_theme->m_transparent);

        nvg::BeginPath(ctx);
        nvg::Rect(ctx, m_pos.x - ds, m_pos.y - ds, m_size.width + (2.0f * ds),
                  m_size.height + (2.0f * ds));
        nvg::RoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);
        nvg::PathWinding(ctx, nvg::NVG_HOLE);
        nvg::FillPaint(ctx, shadow_paint);
        nvg::Fill(ctx);

        // Draw window
        nvg::BeginPath(ctx);
        nvg::RoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);

        ds::point<f32> base{ m_pos + ds::point<f32>{ 0.0f, m_anchor_offset } };

        f32 sign = -1.0f;
        if (m_side == Side::Left)
        {
            base.x += m_size.width;
            sign = 1.0f;
        }

        nvg::MoveTo(ctx, base.x + m_anchor_size * sign, base.y);
        nvg::LineTo(ctx, base.x - (1.0f * sign), base.y - m_anchor_size);
        nvg::LineTo(ctx, base.x - (1.0f * sign), base.y + m_anchor_size);

        nvg::FillColor(ctx, m_theme->m_window_popup);
        nvg::Fill(ctx);
        nvg::Restore(ctx);

        ui::Widget::draw(ctx);
    }
}
