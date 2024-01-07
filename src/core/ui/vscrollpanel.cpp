#include "core/ui/theme.hpp"
#include "core/ui/vscrollpanel.hpp"

namespace rl::gui {

    VScrollPanel::VScrollPanel(ui::widget* parent)
        : ui::widget{ parent }
        , m_child_preferred_height{ 0 }
        , m_scroll{ 0.0f }
        , m_update_layout{ false }
    {
    }

    f32 VScrollPanel::scroll() const
    {
        /**
         * Return the current scroll amount as a value between 0 and 1. 0 means
         * scrolled to the top and 1 to the bottom.
         */
        return m_scroll;
    }

    void VScrollPanel::set_scroll(f32 scroll)
    {
        /**
         * Set the scroll amount to a value between 0 and 1. 0 means scrolled to
         * the top and 1 to the bottom.
         */
        m_scroll = scroll;
    }

    void VScrollPanel::perform_layout(NVGcontext* nvg_context)
    {
        ui::widget::perform_layout(nvg_context);

        if (m_children.empty())
            return;

        runtime_assert(m_children.size() == 0, "vertical scroll panel should only have 1 child");

        ui::widget* child{ m_children[0] };
        m_child_preferred_height = child->preferred_size(nvg_context).height;

        if (m_child_preferred_height > m_size.height)
        {
            child->set_position({
                0,
                static_cast<i32>(-m_scroll * (m_child_preferred_height - m_size.height)),
            });

            child->set_size({
                m_size.width - 12,
                m_child_preferred_height,
            });
        }
        else
        {
            child->set_position(ds::point<i32>{ 0, 0 });
            child->set_size(m_size);
            m_scroll = 0;
        }
        child->perform_layout(nvg_context);
    }

    ds::dims<i32> VScrollPanel::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_children.empty())
            return ds::dims<i32>{ 0, 0 };
        return m_children[0]->preferred_size(nvg_context) + ds::dims<i32>{ 12, 0 };
    }

    bool VScrollPanel::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        if (m_children.empty() || m_child_preferred_height <= m_size.height)
            return ui::widget::on_mouse_drag(mouse, kb);
        else
        {
            f32 scrollh{
                this->height() *
                    std::min(1.f, this->height() / static_cast<f32>(m_child_preferred_height)),
            };

            auto&& rel{ mouse.pos_delta() };

            m_scroll = std::max(
                0.0f, std::min(1.0f, m_scroll + rel.y / (m_size.height - 8.0f - scrollh)));

            m_update_layout = true;
            return true;
        }
    }

    bool VScrollPanel::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if (ui::widget::on_mouse_button_released(mouse, kb))
            return true;

        return false;
    }

    bool VScrollPanel::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        if (ui::widget::on_mouse_button_pressed(mouse, kb))
            return true;

        const auto&& pos{ mouse.pos() };
        if (mouse.is_button_down(Mouse::Button::Left) && !m_children.empty() &&
            m_child_preferred_height > m_size.height && pos.x > m_pos.x + m_size.width - 13 &&
            pos.x < m_pos.x + m_size.width - 4)
        {
            i32 scrollh{ static_cast<i32>(
                this->height() *
                std::min(1.0f, this->height() / static_cast<f32>(m_child_preferred_height))) };

            i32 start{ static_cast<int>(
                m_pos.y + 4 + 1 + (m_size.height - 8 - scrollh) * m_scroll) };

            f32 delta{ 0.0f };
            if (pos.y < start)
                delta = -m_size.height / static_cast<f32>(m_child_preferred_height);
            else if (pos.y > start + scrollh)
                delta = m_size.height / static_cast<f32>(m_child_preferred_height);

            m_scroll = std::max(0.0f, std::min(1.0f, m_scroll + delta * 0.98f));

            m_children[0]->set_position(ds::point<i32>{
                0,
                static_cast<i32>(-m_scroll * (m_child_preferred_height - m_size.height)),
            });
            m_update_layout = true;
            return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        if (m_children.empty() || m_child_preferred_height <= m_size.height)
            return ui::widget::on_mouse_scroll(mouse, kb);
        else
        {
            auto child{ m_children[0] };
            f32 scroll_amount{
                mouse.wheel().y * m_size.height * 0.25f,
            };

            m_scroll = std::max(0.f,
                                std::min(1.f, m_scroll - scroll_amount / m_child_preferred_height));

            ds::point<i32> old_pos{ child->position() };
            child->set_position({
                0,
                static_cast<i32>(-m_scroll * (m_child_preferred_height - m_size.height)),
            });

            ds::point<i32> new_pos{ child->position() };
            m_update_layout = true;

            child->on_mouse_move(mouse, kb);

            return true;
        }
    }

    void VScrollPanel::draw(NVGcontext* ctx)
    {
        if (m_children.empty())
            return;

        ui::widget* child{ m_children[0] };

        i32 yoffset{ 0 };
        if (m_child_preferred_height > m_size.height)
            yoffset = -m_scroll * (m_child_preferred_height - m_size.height);

        child->set_position(ds::point<i32>{ 0, yoffset });
        m_child_preferred_height = child->preferred_size(ctx).height;
        f32 scrollh{ height() *
                     std::min(1.f, height() / static_cast<f32>(m_child_preferred_height)) };

        if (m_update_layout)
        {
            m_update_layout = false;
            child->perform_layout(ctx);
        }

        nvgSave(ctx);
        nvgTranslate(ctx, m_pos.x, m_pos.y);
        nvgIntersectScissor(ctx, 0, 0, m_size.width, m_size.height);

        if (child->visible())
            child->draw(ctx);

        nvgRestore(ctx);

        if (m_child_preferred_height <= m_size.height)
            return;

        NVGpaint paint{ nvgBoxGradient(ctx, m_pos.x + m_size.width - 12 + 1, m_pos.y + 4 + 1, 8,
                                       m_size.height - 8, 3, 4, ds::color<u8>{ 0, 0, 0, 32 },
                                       ds::color<u8>{ 0, 0, 0, 92 }) };
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x + m_size.width - 12, m_pos.y + 4, 8, m_size.height - 8, 3);
        nvgFillPaint(ctx, paint);
        nvgFill(ctx);

        paint = nvgBoxGradient(ctx, m_pos.x + m_size.width - 12 - 1,
                               m_pos.y + 4 + (m_size.height - 8 - scrollh) * m_scroll - 1, 8,
                               scrollh, 3, 4, ds::color<u8>{ 220, 220, 220, 100 },
                               ds::color<u8>{ 128, 128, 128, 100 });

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x + m_size.width - 12 + 1,
                       m_pos.y + 4 + 1 + (m_size.height - 8 - scrollh) * m_scroll, 8 - 2,
                       scrollh - 2, 2);

        nvgFillPaint(ctx, paint);
        nvgFill(ctx);
    }
}
