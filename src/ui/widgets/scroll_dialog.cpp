#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include "ui/canvas.hpp"
#include "ui/widgets/scroll_dialog.hpp"
#include "utils/conversions.hpp"

namespace rl::ui {
    std::string ScrollableDialog::title() const
    {
        return m_title;
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
        if (m_rect.contains(pt)) {
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
        // todo: clean this up
        m_enabled_interactions &= ~inter;
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

    bool ScrollableDialog::on_mouse_button_pressed(const Mouse&, const Keyboard&)
    {
        return false;
    }

    bool ScrollableDialog::on_mouse_button_released(const Mouse&, const Keyboard&)
    {
        return false;
    }

    bool ScrollableDialog::on_mouse_scroll(const Mouse&, const Keyboard&)
    {
        return false;
    }

    bool ScrollableDialog::on_mouse_drag(const Mouse&, const Keyboard&)
    {
        return false;
    }

    void ScrollableDialog::draw()
    {
        const auto context{ m_renderer->context() };
        const f32 drop_shadow_size{ m_theme->dialog_drop_shadow_size };
        const f32 corner_radius{ m_theme->dialog_corner_radius };
        const f32 header_height{ this->header_height() };

        m_renderer->scoped_draw([&] {
            m_renderer->draw_path(false, [&] {
                nvg::rounded_rect(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width,
                                  m_rect.size.height, corner_radius);
                nvg::fill_color(context, m_mouse_focus ? m_theme->dialog_fill_focused
                                                       : m_theme->dialog_fill_unfocused);
                nvg::fill(context);
            });

            // Dialog shadow
            m_renderer->scoped_draw([&] {
                m_renderer->reset_scissor();
                m_renderer->draw_path(false, [&] {
                    const nvg::PaintStyle shadow_paint{
                        nvg::box_gradient(
                            context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width, m_rect.size.height,
                            corner_radius * 2.0f, drop_shadow_size * 2.0f, m_theme->dialog_shadow,
                            m_theme->transparent)
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

            if (!m_title.empty()) {
                m_renderer->draw_path(false, [&] {
                    const nvg::PaintStyle header_style{
                        nvg::linear_gradient(
                            context, m_rect.pt.x, m_rect.pt.y, m_rect.pt.x, m_rect.pt.y + header_height,
                            m_theme->dialog_header_gradient_top, m_theme->dialog_header_gradient_bot)
                    };

                    m_renderer->draw_rounded_rect(
                        ds::rect<f32>{
                            ds::point<f32>{ m_rect.pt },
                            ds::dims<f32>{ m_rect.size.width, header_height } },
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

                nvg::set_font_size(context, m_theme->tooltip_font_size);
                nvg::set_font_face(context, m_theme->tooltip_font_name.data());
                nvg::set_text_align(context, Align::HCenter | Align::VMiddle);

                // header text shadow
                nvg::font_blur_(context, 2.0f);
                nvg::fill_color(context, m_theme->text_shadow);
                nvg::draw_text(
                    context,
                    ds::point<f32>{
                        m_rect.pt.x + (m_rect.size.width / 2.0f),
                        m_rect.pt.y + (header_height / 2.0f),
                    },
                    m_title);

                // Header text
                nvg::font_blur_(context, 0.0f);
                nvg::fill_color(context, m_focused ? m_theme->dialog_title_focused
                                                   : m_theme->dialog_title_unfocused);
                nvg::draw_text(
                    context,
                    ds::point<f32>{
                        m_rect.pt.x + (m_rect.size.width / 2.0f),
                        m_rect.pt.y + (header_height / 2.0f) - 1.0f,
                    },
                    m_title);
            }
        });

        Widget::draw();
    }

    // void ScrollableDialog::perform_layout()
    //{
    // }

    ds::dims<f32> ScrollableDialog::preferred_size() const
    {
        return ds::dims<f32>::zero();
    }

    void ScrollableDialog::refresh_relative_placement()
    {
    }

}
