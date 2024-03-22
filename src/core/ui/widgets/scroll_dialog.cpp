#include <memory>
#include <string>
#include <tuple>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/widgets/scroll_dialog.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"

namespace rl::ui {

    ScrollableDialog::ScrollableDialog(Widget* parent, std::string title)
        : Widget{ parent }
        , m_title{ std::move(title) }
    {
    }

    std::string ScrollableDialog::title() const
    {
        return m_title;
    }

    std::tuple<Interaction, Component, Side> ScrollableDialog::check_interaction(
        const ds::point<f32>& pt) const
    {
        // default to no interaction / interaction
        // candidate components of the dialog
        std::tuple ret{
            Interaction::None,
            Component::None,
            m_rect.edge_overlap(RESIZE_GRAB_BUFFER, pt),
        };

        // check if the mouse is above a grab point
        auto& [interaction, component, grab_edge] = ret;
        const bool resizeable{ (m_enabled_interactions & Interaction::Resize) != Interaction::None };
        if (resizeable && grab_edge != Side::None)
        {
            interaction = Interaction::Resize;
            component = Component::Edge;
            return ret;
        }

        // check if the mouse cursor is overlapping with the
        // dialog's header / title bar that can be grabbed
        const bool moveable{ (m_enabled_interactions & Interaction::Move) != Interaction::None };
        if (moveable && m_header_visible)
        {
            const f32 header_height{ this->header_height() };
            ds::rect header_rect{
                m_rect.pt,
                ds::dims{
                    m_rect.size.width,
                    header_height,
                },
            };

            if (header_rect.contains(pt))
            {
                interaction = Interaction::Move;
                component = Component::Header;
                return ret;
            }
        }

        // check to see if mouse is hovering
        // above the scrollbar when it's visible
        if (m_scrollbar_visible)
        {
            const ds::rect scrollbar_rect{
                ds::point{
                    m_rect.pt.x + m_rect.size.width - (SDScrollbarWidth + SDMargin),
                    m_rect.pt.y,
                },
                ds::dims{
                    SDScrollbarWidth,
                    m_rect.size.height,
                },
            };

            if (scrollbar_rect.contains(pt))
            {
                interaction = Interaction::Drag;
                component = Component::Scrollbar;
                return ret;
            }
        }

        // check if the mouse cursor falls within
        // the widget container body of the dialog
        if (m_rect.contains(pt))
        {
            interaction = Interaction::Propagate;
            component = Component::Body;
            return ret;
        }

        return ret;
    }

    f32 ScrollableDialog::header_height() const
    {
        if (!m_title.empty())
            return m_theme->dialog_header_height;

        return 0.0f;
    }

    f32 ScrollableDialog::scroll_pos() const
    {
        return m_scrollbar_position;
    }

    void ScrollableDialog::set_scroll_pos(const f32 pos)
    {
        runtime_assert(pos >= 0.0f && pos <= 1.0f, "invalid scrollbar pos");
        m_scrollbar_position = pos;
    }

    bool ScrollableDialog::interaction_enabled(const Interaction inter) const
    {
        return (m_enabled_interactions & inter) != 0;
    }

    void ScrollableDialog::enable_interaction(Interaction inter)
    {
        m_enabled_interactions |= inter;
    }

    void ScrollableDialog::disable_interaction(Interaction inter)
    {
        // todo: clean this up
        m_enabled_interactions &= static_cast<Interaction>(~std::to_underlying(inter));
    }

    bool ScrollableDialog::mode_active(const Interaction inter) const
    {
        if (!this->interaction_enabled(inter))
            return false;

        return (m_active_interactions & inter) != 0;
    }

    Widget* ScrollableDialog::button_panel() const
    {
        return nullptr;
    }

    void ScrollableDialog::set_title(std::string title)
    {
        m_header_visible = !title.empty();
        m_title = std::move(title);
    }

    void ScrollableDialog::center()
    {
        Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        dynamic_cast<Canvas*>(owner)->center_dialog(this);
    }

    void ScrollableDialog::dispose()
    {
        Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        dynamic_cast<Canvas*>(owner)->dispose_dialog(this);
    }

    bool ScrollableDialog::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        return false;
    }

    bool ScrollableDialog::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        return false;
    }

    bool ScrollableDialog::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        return false;
    }

    bool ScrollableDialog::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        return false;
    }

    void ScrollableDialog::draw()
    {
    }

    void ScrollableDialog::perform_layout()
    {
    }

    ds::dims<f32> ScrollableDialog::preferred_size() const
    {
        return {};
    }

    void ScrollableDialog::refresh_relative_placement()
    {
    }

}
