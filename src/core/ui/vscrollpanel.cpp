#include "core/ui/theme.hpp"
#include "core/ui/vscrollpanel.hpp"

namespace rl::ui {

    VScrollPanel::VScrollPanel(Widget* parent)
        : Widget{ parent }
        , m_child_preferred_height{ 0 }
        , m_scroll{ 0.0f }
        , m_update_layout{ false }
    {
    }

    f32 VScrollPanel::scroll() const
    {
        // Return the current scroll amount as a value between 0 and 1. 0 means
        // scrolled to the top and 1 to the bottom.
        return m_scroll;
    }

    void VScrollPanel::set_scroll(f32 scroll)
    {
        // Set the scroll amount to a value between 0 and 1. 0 means scrolled to
        // the top and 1 to the bottom.
        m_scroll = scroll;
    }

    void VScrollPanel::perform_layout(nvg::NVGcontext* nvg_context)
    {
        Widget::perform_layout(nvg_context);

        if (m_children.empty())
            return;

        runtime_assert(m_children.size() == 1, "vertical scroll panel should only have 1 child");

        Widget* child{ m_children[0] };
        m_child_preferred_height = child->preferred_size(nvg_context).height;

        if (m_child_preferred_height > m_size.height)
        {
            child->set_position(ds::point<f32>{
                0.0f,
                -m_scroll * (m_child_preferred_height - m_size.height),
            });

            child->set_size(ds::dims<f32>{
                m_size.width - 12.0f,
                m_child_preferred_height,
            });
        }
        else
        {
            child->set_position(ds::point<f32>{ 0.0f, 0.0f });
            child->set_size(m_size);
            m_scroll = 0;
        }

        child->perform_layout(nvg_context);
    }

    ds::dims<f32> VScrollPanel::preferred_size(nvg::NVGcontext* nvg_context) const
    {
        if (m_children.empty())
            return ds::dims<f32>{ 0.0f, 0.0f };

        return m_children[0]->preferred_size(nvg_context) + ds::dims<f32>{ 12.0f, 0.0f };
    }

    bool VScrollPanel::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        if (m_children.empty() || m_child_preferred_height <= m_size.height)
            return Widget::on_mouse_drag(mouse, kb);
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
        if (Widget::on_mouse_button_released(mouse, kb))
            return true;

        return false;
    }

    bool VScrollPanel::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        const auto&& pos{ mouse.pos() };
        if (mouse.is_button_down(Mouse::Button::Left) && !m_children.empty() &&
            m_child_preferred_height > m_size.height && pos.x > m_pos.x + m_size.width - 13 &&
            pos.x < m_pos.x + m_size.width - 4)
        {
            i32 scrollh{ static_cast<i32>(
                this->height() *
                std::min(1.0f, this->height() / static_cast<f32>(m_child_preferred_height))) };

            i32 start{ static_cast<i32>(
                m_pos.y + 4 + 1 + (m_size.height - 8 - scrollh) * m_scroll) };

            f32 delta{ 0.0f };
            if (pos.y < start)
                delta = -m_size.height / static_cast<f32>(m_child_preferred_height);
            else if (pos.y > start + scrollh)
                delta = m_size.height / static_cast<f32>(m_child_preferred_height);

            m_scroll = std::max(0.0f, std::min(1.0f, m_scroll + delta * 0.98f));
            m_children[0]->set_position(ds::point<i32>{
                0, static_cast<i32>(-m_scroll * (m_child_preferred_height - m_size.height)) });

            m_update_layout = true;
            return true;
        }

        return false;
    }

    bool VScrollPanel::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        if (m_children.empty() || m_child_preferred_height <= m_size.height)
            return Widget::on_mouse_scroll(mouse, kb);
        else
        {
            Widget* child{ m_children[0] };
            f32 scroll_amount{ mouse.wheel().y * m_size.height * 0.25f };

            m_scroll = std::max(0.0f,
                                std::min(1.f, m_scroll - scroll_amount / m_child_preferred_height));

            ds::point<f32> old_pos{ child->position() };

            child->set_position(ds::point<f32>{
                0.0f,
                -m_scroll * (m_child_preferred_height - m_size.height),
            });

            ds::point<f32> new_pos{ child->position() };

            m_update_layout = true;
            child->on_mouse_move(mouse, kb);

            return true;
        }
    }

    void VScrollPanel::draw(nvg::NVGcontext* nvg_context)
    {
        if (m_children.empty())
            return;

        Widget* child{ m_children[0] };

        f32 yoffset{ 0.0f };
        if (m_child_preferred_height > m_size.height)
            yoffset = -m_scroll * (m_child_preferred_height - m_size.height);

        child->set_position(ds::point<f32>{ 0.0f, yoffset });
        m_child_preferred_height = child->preferred_size(nvg_context).height;
        f32 scrollh{ this->height() * std::min(1.0f, this->height() / m_child_preferred_height) };

        if (m_update_layout)
        {
            m_update_layout = false;
            child->perform_layout(nvg_context);
        }

        nvg::Save(nvg_context);
        nvg::Translate(nvg_context, m_pos.x, m_pos.y);
        nvg::IntersectScissor(nvg_context, 0.0f, 0.0f, m_size.width, m_size.height);

        if (child->visible())
            child->draw(nvg_context);

        nvg::Restore(nvg_context);

        if (m_child_preferred_height <= m_size.height)
            return;

        nvg::NVGpaint paint{ nvg::BoxGradient(nvg_context, m_pos.x + m_size.width - 12.0f + 1.0f,
                                              m_pos.y + 4.0f + 1.0f, 8.0f, m_size.height - 8.0f,
                                              3.0f, 4.0f, ds::color<f32>{ 0, 0, 0, 32 },
                                              ds::color<f32>{ 0, 0, 0, 92 }) };
        nvg::BeginPath(nvg_context);
        nvg::RoundedRect(nvg_context, m_pos.x + m_size.width - 12, m_pos.y + 4, 8,
                         m_size.height - 8, 3);
        nvg::FillPaint(nvg_context, paint);
        nvg::Fill(nvg_context);

        paint = nvg::BoxGradient(
            nvg_context, m_pos.x + m_size.width - 12.0f - 1.0f,
            m_pos.y + 4.0f + (m_size.height - 8.0f - scrollh) * m_scroll - 1.0f, 8.0f, scrollh,
            3.0f, 4.0f, ds::color<f32>{ 220, 220, 220, 100 }, ds::color<f32>{ 128, 128, 128, 100 });

        nvg::BeginPath(nvg_context);
        nvg::RoundedRect(nvg_context, m_pos.x + m_size.width - 1.0f + 1.0f,
                         m_pos.y + 4.0f + 1.0f + (m_size.height - 8.0f - scrollh) * m_scroll,
                         8.0f - 2.0f, scrollh - 2.0f, 2.0f);

        nvg::FillPaint(nvg_context, paint);
        nvg::Fill(nvg_context);
    }
}
