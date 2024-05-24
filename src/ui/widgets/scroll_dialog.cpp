#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include "gfx/vg/nanovg_state.hpp"
#include "panel.hpp"
#include "ui/canvas.hpp"
#include "ui/widgets/scroll_dialog.hpp"
#include "utils/conversions.hpp"

namespace rl::ui {
    ScrollableDialog::ScrollableDialog(std::string title, const ds::dims<f32> fixed_size)
        : Widget{ nullptr }
        , m_title{ std::move(title) }
    {
        this->set_resizable(true);
        this->set_icon_extra_scale(0.8f);
        if (fixed_size.valid())
            Widget::set_size(fixed_size);

        // horizontally aligns title (centered),
        // minimize, maximize, and close buttons
        const auto dgtitle{ new Label{ "Dialog Title", 24, Align::HLeft | Align::VMiddle } };
        const auto min_btn{ new Button{ Icon::WindowMinimize } };
        const auto max_btn{ new Button{ Icon::WindowMaximize } };
        const auto cls_btn{ new Button{ Icon::WindowClose } };

        dgtitle->set_expansion(20.0f);
        min_btn->set_font_size(18.0f);
        max_btn->set_font_size(18.0f);
        cls_btn->set_font_size(18.0f);

        m_titlebar_layout = new BoxLayout<Alignment::Horizontal>{
            "Buttons Horiz",
            {
                dgtitle,
                min_btn,
                max_btn,
                cls_btn,
            },
        };

        m_titlebar_layout->set_size_policy(SizePolicy::Minimum);
        m_titlebar_layout->set_margins({ 0.0f }, { 1.0f });
        m_titlebar_layout->set_spacing(2.0f);

        // horizontally aligns the contents panel
        // containing all children, and scrollbar
        m_scrollbar_panel = new Label{ "" };
        m_scrollbar_panel->set_fixed_width(250.0f);
        m_scrollbar_panel->set_expansion(0.025f);
        m_body_layout = new BoxLayout<Alignment::Horizontal>{
            "Body Horiz",
            {
                new Label{ "Body", -1.0f, Align::HCenter | Align::VMiddle },
                m_scrollbar_panel,
            },
        };

        m_body_layout->set_margins({ 0.0f }, { 1.0f });
        m_body_layout->set_spacing(0.0f);

        // vertically aligns titlebar and dialog body
        m_root_layout = new BoxLayout<Alignment::Vertical>{ "Dialog Root Vert" };
        m_root_layout->set_margins({ 0.0f }, { 1.0f });
        m_root_layout->set_size_policy(SizePolicy::Maximum);
        m_root_layout->add_nested_layout(m_titlebar_layout);
        m_root_layout->add_nested_layout(m_body_layout);

        this->assign_layout(m_root_layout);
        Widget::perform_layout();
    }

    std::tuple<Interaction, Component, Side> ScrollableDialog::check_interaction(const ds::point<f32> pt) const
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
        if (resizeable && grab_edge != Side::None) {
            interaction = Interaction::Resize;
            component = Component::Edge;
            return ret;
        }

        // check if the mouse cursor is overlapping with the
        // dialog's header / title bar that can be grabbed
        const bool moveable{ (m_enabled_interactions & Interaction::Move) != Interaction::None };
        if (moveable && m_header_visible) {
            const f32 header_height{ this->header_height() };
            const ds::rect header_rect{
                m_rect.pt,
                ds::dims{
                    m_rect.size.width,
                    header_height,
                },
            };

            if (header_rect.contains(pt)) {
                interaction = Interaction::Move;
                component = Component::Header;
                return ret;
            }
        }

        // check to see if mouse is hovering
        // above the scrollbar when it's visible
        if (m_scrollbar_visible) {
            const ds::rect<f32> scrollbar_rect{
                ds::point<f32>{
                    m_rect.pt.x + m_rect.size.width - (SDScrollbarWidth + SDMargin),
                    m_rect.pt.y,
                },
                ds::dims<f32>{
                    SDScrollbarWidth,
                    m_rect.size.height,
                },
            };

            if (scrollbar_rect.contains(pt)) {
                interaction = Interaction::Drag;
                component = Component::Scrollbar;
                return ret;
            }
        }

        // check if the mouse cursor falls within
        // the widget container body of the dialog
        if (m_body_layout->contains(pt)) {
            interaction = Interaction::Propagate;
            component = Component::Body;
            return ret;
        }

        return ret;
    }

    std::string_view ScrollableDialog::title() const
    {
        return m_title;
    }

    f32 ScrollableDialog::header_height() const
    {
        const auto titlebar_rect{ m_titlebar_layout->rect() };
        if (titlebar_rect.valid() && !m_title.empty())
            // TODO: use margin values instead of 4.0f
            return m_body_layout->rect().top() + 4.0f;

        return 0.0f;
    }

    f32 ScrollableDialog::scroll_pos() const
    {
        return m_scrollbar_position;
    }

    void ScrollableDialog::set_scroll_pos(const f32 pos)
    {
        debug_assert(pos >= 0.0f && pos <= 1.0f, "invalid scrollbar pos");
        m_scrollbar_position = pos;
    }

    bool ScrollableDialog::interaction_enabled(const Interaction inter) const
    {
        return (m_enabled_interactions & inter) != 0;
    }

    void ScrollableDialog::enable_interaction(const Interaction inter)
    {
        m_enabled_interactions |= inter;
    }

    void ScrollableDialog::disable_interaction(const Interaction inter)
    {
        m_enabled_interactions &= ~inter;
    }

    bool ScrollableDialog::mode_active(const Interaction inter) const
    {
        if (!this->interaction_enabled(inter))
            return false;

        return (m_active_interactions & inter) != 0;
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

    bool ScrollableDialog::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb, ds::point<f32>)
    {
        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        switch (m_mode) {
            case DialogMode::Move:
            {
                LocalTransform dialog_transform{ this };
                LocalTransform root_transform{ m_root_layout };
                LocalTransform titlebar_transform{ m_titlebar_layout };
                const ds::point<f32> local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
                return m_titlebar_layout->contains(local_mouse_pos);
            }
            case DialogMode::Resize:
            {
                m_resize_grab_location = m_rect.edge_overlap(RESIZE_GRAB_BUFFER, mouse.pos());
                debug_assert(m_resize_grab_location != Side::None,
                             "dialog resizing without grab location");
                return m_resize_grab_location != Side::None;
            }
            case DialogMode::None:
            {
                // const Side border_grab{ m_rect.edge_overlap(RESIZE_GRAB_BUFFER, mouse.pos()) };
                // m_resize_grab_location = border_grab;
                return false;
            }
            case DialogMode::Modal:
                return false;
        }

        return false;
    }

    bool ScrollableDialog::on_mouse_drag(const Mouse& mouse, const Keyboard&)
    {
        switch (m_mode) {
            case DialogMode::Move:
            {
                const bool move_btn_down{ mouse.is_button_down(Mouse::Button::Left) };
                if (move_btn_down) {
                    m_rect.pt += mouse.pos_delta();
                    m_rect.pt.x = std::max(m_rect.pt.x, 0.0f);
                    m_rect.pt.y = std::max(m_rect.pt.y, 0.0f);

                    const ds::dims relative_size{ this->parent()->size() - m_rect.size };
                    m_rect.pt.x = std::min(m_rect.pt.x, relative_size.width);
                    m_rect.pt.y = std::min(m_rect.pt.y, relative_size.height);
                    return true;
                }
                break;
            }
            case DialogMode::Resize:
            {
                const bool resize_btn_down{ mouse.is_button_down(Mouse::Button::Left) };
                if (resize_btn_down) {
                    const auto delta{ mouse.pos_delta() };
                    switch (m_resize_grab_location) {
                        case Side::Top:
                            m_rect.pt.y += delta.y;
                            m_rect.size.height -= delta.y;
                            this->perform_layout();
                            return true;
                        case Side::Bottom:
                            m_rect.size.height += delta.y;
                            this->perform_layout();
                            return true;
                        case Side::Left:
                            m_rect.pt.x += delta.x;
                            m_rect.size.width -= delta.x;
                            this->perform_layout();
                            return true;
                        case Side::Right:
                            m_rect.size.width += delta.x;
                            this->perform_layout();
                            return true;
                        case Side::TopLeft:
                            m_rect.pt.x += delta.x;
                            m_rect.pt.y += delta.y;
                            m_rect.size.width -= delta.x;
                            m_rect.size.height -= delta.y;
                            this->perform_layout();
                            return true;
                        case Side::TopRight:
                            m_rect.pt.y += delta.y;
                            m_rect.size.width += delta.x;
                            m_rect.size.height -= delta.y;
                            this->perform_layout();
                            return true;
                        case Side::BottomLeft:
                            m_rect.pt.x += delta.x;
                            m_rect.size.width -= delta.x;
                            m_rect.size.height += delta.y;
                            this->perform_layout();
                            return true;
                        case Side::BottomRight:
                            m_rect.size.width += delta.x;
                            m_rect.size.height += delta.y;
                            this->perform_layout();
                            return true;

                        default:
                            debug_assert("Invalid/unhandled resize grab location");
                            [[fallthrough]];
                        case Side::None:
                            break;
                    }
                }

                break;
            }

            default:
                debug_assert("Invalid/unsupported DialogMode");
                [[fallthrough]];
            case DialogMode::None:
                [[fallthrough]];
            case DialogMode::Modal:
                break;
        }

        return false;
    }

    bool ScrollableDialog::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        m_mode = DialogMode::None;
        m_resize_grab_location = Side::None;
        if (Widget::on_mouse_button_released(mouse, kb))
            return true;

        // if ((m_enabled_interactions & (Interaction::Move | Interaction::Scroll)) == 0)
        //     return false;
        // if (mouse.button_released() != Mouse::Button::Left)
        //     return false;

        // m_active_interactions &= ~Interaction::Move;
        // m_active_interactions &= ~Interaction::Scroll;
        return false;
    }

    bool ScrollableDialog::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_scroll(mouse, kb);
        return true;
    }

    void ScrollableDialog::draw()
    {
        const auto context{ m_renderer->context() };
        const f32 drop_shadow_size{ m_theme->dialog_drop_shadow_size };
        const f32 corner_radius{ m_theme->dialog_corner_radius };
        const f32 header_height{ this->header_height() };

        m_renderer->scoped_draw([&] {
            m_renderer->draw_path(false, [&] {
                m_renderer->draw_rounded_rect(m_rect, corner_radius);
                nvg::rounded_rect(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width, m_rect.size.height, corner_radius);
                nvg::fill_color(context, m_mouse_focus ? m_theme->dialog_fill_focused
                                                       : m_theme->dialog_fill_unfocused);
                nvg::fill(context);
            });

            // Dialog shadow
            m_renderer->scoped_draw([&] {
                m_renderer->reset_scissor();
                m_renderer->draw_path(false, [&] {
                    const nvg::PaintStyle shadow_paint{
                        m_renderer->create_rect_gradient_paint_style(
                            m_rect, corner_radius * 2.0f, drop_shadow_size * 2.0f,
                            m_theme->dialog_shadow, m_theme->transparent)
                    };

                    nvg::rect(context, m_rect.pt.x - drop_shadow_size,
                              m_rect.pt.y - drop_shadow_size,
                              m_rect.size.width + 2.0f * drop_shadow_size,
                              m_rect.size.height + 2.0f * drop_shadow_size);

                    nvg::rounded_rect(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width,
                                      m_rect.size.height, corner_radius);
                    nvg::path_winding(context, nvg::Solidity::Hole);
                    nvg::fill_paint(context, shadow_paint);
                    nvg::fill(context);
                });
            });

            if (header_height > 0.0f) {
                m_renderer->draw_path(false, [&] {
                    const nvg::PaintStyle header_style{
                        nvg::linear_gradient(
                            context, m_rect.pt.x, m_rect.pt.y, m_rect.pt.x, m_rect.pt.y + header_height,
                            m_theme->dialog_header_gradient_top, m_theme->dialog_header_gradient_bot)
                    };

                    m_renderer->draw_rounded_rect(
                        ds::rect<f32>{ m_rect.pt, ds::dims{ m_rect.size.width, header_height } },
                        corner_radius);
                    m_renderer->fill_current_path(header_style);
                });

                m_renderer->draw_path(false, [&] {
                    m_renderer->draw_rounded_rect(
                        ds::rect{ m_rect.pt, ds::dims{ m_rect.size.width, header_height } },
                        corner_radius);

                    nvg::stroke_color(context, m_theme->dialog_header_sep_top);

                    m_renderer->scoped_draw([&] {
                        nvg::intersect_scissor(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width, 0.5f);
                        nvg::stroke(context);
                    });
                });

                m_renderer->draw_path(false, [&] {
                    nvg::move_to(context, m_rect.pt.x + 0.5f, m_rect.pt.y + header_height - 1.5f);
                    nvg::line_to(context, m_rect.pt.x + m_rect.size.width - 0.5f,
                                 m_rect.pt.y + header_height - 1.5f);
                    nvg::stroke_color(context, m_theme->dialog_header_sep_bot);
                    nvg::stroke(context);
                });
            }
        });

        Widget::draw();
    }

    ds::dims<f32> ScrollableDialog::preferred_size() const
    {
        debug_assert("not implemented");
        return {};
    }

    DialogMode ScrollableDialog::mode() const
    {
        return m_mode;
    }

    void ScrollableDialog::set_mode(const DialogMode mode)
    {
        m_mode = mode;
    }

    void ScrollableDialog::refresh_relative_placement()
    {
        // derived classes should define
        return;
    }
}
