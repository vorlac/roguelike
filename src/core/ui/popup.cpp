#include <nanovg.h>

#include "core/ui/popup.hpp"
#include "core/ui/theme.hpp"

namespace rl::ui {

    Popup::Popup(ui::widget* parent, ui::Dialog* parent_window)
        : ui::Dialog{ parent, "" }
        , m_parent_window{ parent_window }
        , m_anchor_pos{ 0, 0 }
        , m_anchor_offset{ 30 }
        , m_anchor_size{ 15 }
        , m_side{ Side::Right }
    {
    }

    void Popup::set_anchor_pos(const ds::point<i32>& anchor_pos)
    {
        m_anchor_pos = anchor_pos;
    }

    const ds::point<i32> Popup::anchor_pos() const
    {
        return m_anchor_pos;
    }

    void Popup::set_anchor_offset(i32 anchor_offset)
    {
        m_anchor_offset = anchor_offset;
    }

    i32 Popup::anchor_offset() const
    {
        return m_anchor_offset;
    }

    void Popup::set_anchor_size(i32 anchor_size)
    {
        m_anchor_size = anchor_size;
    }

    i32 Popup::anchor_size() const
    {
        return m_anchor_size;
    }

    void Popup::set_side(Side popup_side)
    {
        m_side = popup_side;
    }

    Popup::Side Popup::side() const
    {
        return m_side;
    }

    ui::Dialog* Popup::parent_window()
    {
        return m_parent_window;
    }

    const ui::Dialog* Popup::parent_window() const
    {
        return m_parent_window;
    }

    void Popup::perform_layout(NVGcontext* ctx)
    {
        if (m_layout != nullptr || m_children.size() != 1)
            ui::widget::perform_layout(ctx);
        else
        {
            m_children[0]->set_position({ 0, 0 });
            m_children[0]->set_size(m_size);
            m_children[0]->perform_layout(ctx);
        }

        if (m_side == Side::Left)
            m_anchor_pos.x -= size().width;
    }

    void Popup::refresh_relative_placement()
    {
        if (!m_parent_window)
            return;

        m_parent_window->refresh_relative_placement();
        m_visible &= m_parent_window->visible_recursive();
        m_pos = m_parent_window->position() + m_anchor_pos - ds::point<i32>{ 0, m_anchor_offset };
    }

    void Popup::draw(NVGcontext* ctx)
    {
        this->refresh_relative_placement();

        if (!m_visible)
            return;

        i32 ds{ m_theme->m_window_drop_shadow_size };
        i32 cr{ m_theme->m_window_corner_radius };

        nvgSave(ctx);
        nvgResetScissor(ctx);

        // Draw a drop shadow
        NVGpaint shadow_paint = nvgBoxGradient(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height,
                                               cr * 2, ds * 2, m_theme->m_drop_shadow,
                                               m_theme->m_transparent);

        nvgBeginPath(ctx);
        nvgRect(ctx, m_pos.x - ds, m_pos.y - ds, m_size.width + 2 * ds, m_size.height + 2 * ds);
        nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadow_paint);
        nvgFill(ctx);

        // Draw window
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);

        ds::point<i32> base{ m_pos + ds::point<i32>{ 0, m_anchor_offset } };
        i32 sign = -1;
        if (m_side == Side::Left)
        {
            base.x += m_size.width;
            sign = 1;
        }

        nvgMoveTo(ctx, base.x + m_anchor_size * sign, base.y);
        nvgLineTo(ctx, base.x - 1 * sign, base.y - m_anchor_size);
        nvgLineTo(ctx, base.x - 1 * sign, base.y + m_anchor_size);

        nvgFillColor(ctx, m_theme->m_window_popup);
        nvgFill(ctx);
        nvgRestore(ctx);

        ui::widget::draw(ctx);
    }
}
