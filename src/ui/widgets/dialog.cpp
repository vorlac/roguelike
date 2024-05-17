#include <string>
#include <utility>

#include "ds/dims.hpp"
#include "ds/vector2d.hpp"
#include "gfx/vg/nanovg.hpp"
#include "ui/canvas.hpp"
#include "ui/layouts/box_layout.hpp"
#include "ui/widgets/dialog.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"

namespace rl::ui {
    Dialog::Dialog(Widget* parent, std::string title)
        : Widget{ parent }
        , m_title{ std::move(title) }
    {
        m_resizable = true;
    }

    std::string Dialog::title() const
    {
        return m_title;
    }

    void Dialog::set_title(const std::string& title)
    {
        m_title = title;
    }

    Dialog::Mode Dialog::mode() const
    {
        return m_mode;
    }

    void Dialog::set_mode(const Dialog::Mode mode)
    {
        m_mode = mode;
    }

    Widget* Dialog::button_panel()
    {
        if (m_button_panel == nullptr)
            m_button_panel = new Widget{ this };

        return m_button_panel;
    }

    f32 Dialog::header_height() const
    {
        if (!m_title.empty())
            return m_theme->dialog_header_height;

        return 0.0f;
    }

    void Dialog::draw()
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
                    const nvg::PaintStyle shadow_paint{ nvg::box_gradient(
                        context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width, m_rect.size.height,
                        corner_radius * 2.0f, drop_shadow_size * 2.0f, m_theme->dialog_shadow,
                        m_theme->transparent) };

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
                    nvg::PaintStyle header_style{ nvg::linear_gradient(
                        context, m_rect.pt.x, m_rect.pt.y, m_rect.pt.x, m_rect.pt.y + header_height,
                        m_theme->dialog_header_gradient_top, m_theme->dialog_header_gradient_bot) };

                    m_renderer->draw_rounded_rect(
                        ds::rect<f32>{
                            ds::point<f32>{ m_rect.pt },
                            ds::dims<f32>{ m_rect.size.width, header_height },
                        },
                        corner_radius);

                    m_renderer->fill_current_path(header_style);
                });

                m_renderer->draw_path(false, [&] {
                    m_renderer->draw_rounded_rect(
                        ds::rect{ m_rect.pt, ds::dims{ m_rect.size.width, header_height } },
                        corner_radius);

                    nvg::stroke_color(context, m_theme->dialog_header_sep_top);

                    m_renderer->scoped_draw([&] {
                        nvg::intersect_scissor(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width,
                                               0.5f);
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
                nvg::draw_text(context,
                               ds::point<f32>{
                                   m_rect.pt.x + (m_rect.size.width / 2.0f),
                                   m_rect.pt.y + (header_height / 2.0f),
                               },
                               m_title);

                // Header text
                nvg::font_blur_(context, 0.0f);
                nvg::fill_color(context, m_focused ? m_theme->dialog_title_focused
                                                   : m_theme->dialog_title_unfocused);
                nvg::draw_text(context,
                               ds::point<f32>{
                                   m_rect.pt.x + (m_rect.size.width / 2.0f),
                                   m_rect.pt.y + (header_height / 2.0f) - 1.0f,
                               },
                               m_title);
            }
        });

        Widget::draw();
    }

    void Dialog::set_resize_grab_pos(const Side side)
    {
        m_resize_grab_location = side;
    }

    Side Dialog::resize_side() const
    {
        return m_resize_grab_location;
    }

    bool Dialog::on_mouse_drag(const Mouse& mouse, const Keyboard&)
    {
        switch (m_mode) {
            case Dialog::Mode::Move:
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
            case Dialog::Mode::Resizing:
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
                debug_assert("Invalid/unsupported Dialog::Mode");
                [[fallthrough]];
            case Dialog::Mode::None:
                [[fallthrough]];
            case Dialog::Mode::Modal:
                break;
        }

        return false;
    }

    bool Dialog::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb, ds::point<f32>)
    {
        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        switch (m_mode) {
            case Dialog::Mode::Move:
            {
                const f32 offset_height{ mouse.pos().y - m_rect.pt.y };
                if (offset_height < m_theme->dialog_header_height)
                    return true;
                return false;
            }
            case Dialog::Mode::Resizing:
            {
                // m_resize_grab_location = m_rect.edge_overlap(RESIZE_GRAB_BUFFER, mouse.pos());
                debug_assert(m_resize_grab_location != Side::None,
                             "dialog resizing without grab location");
                return m_resize_grab_location != Side::None;
            }
            case Dialog::Mode::None:
            {
                // const Side border_grab{ m_rect.edge_overlap(RESIZE_GRAB_BUFFER, mouse.pos()) };
                // m_resize_grab_location = border_grab;
                return false;
            }
            case Dialog::Mode::Modal:
                return false;
        }

        return false;
    }

    bool Dialog::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        m_mode = Dialog::Mode::None;
        m_resize_grab_location = Side::None;

        if (Widget::on_mouse_button_released(mouse, kb))
            return true;

        return false;
    }

    bool Dialog::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        Widget::on_mouse_scroll(mouse, kb);
        return true;
    }

    ds::dims<f32> Dialog::preferred_size() const
    {
        if (m_button_panel != nullptr)
            m_button_panel->hide();

        const ds::dims result{ Widget::preferred_size() };

        if (m_button_panel != nullptr)
            m_button_panel->show();

        const auto context{ m_renderer->context() };
        nvg::set_font_size(context, m_theme->dialog_title_font_size);
        nvg::set_font_face(context, m_theme->dialog_title_font_name.data());

        ds::rect bounds{ ds::rect<f32>::zero() };
        nvg::text_bounds(context, ds::point<f32>::zero(), m_title, bounds);

        constexpr static f32 TEXT_SIZE_WIDTH_PADDING{ 20.0f };
        const ds::rect<f32> bounding_rect{
            ds::point<f32>::zero(),
            ds::dims<f32>{
                std::max(result.width, bounds.size.width + TEXT_SIZE_WIDTH_PADDING),
                std::max(result.height, bounds.size.height),
            },
        };

        return bounding_rect.size;
    }

    void Dialog::perform_layout()
    {
        scoped_log();
        if (m_button_panel == nullptr)
            Widget::perform_layout();
        else {
            m_button_panel->hide();

            Widget::perform_layout();
            for (const auto bp_child : m_button_panel->children()) {
                bp_child->set_fixed_size(ds::dims<f32>{ 22.0f, 22.0f });
                bp_child->set_font_size(15.0f);
            }

            m_button_panel->show();
            m_button_panel->set_size(ds::dims<f32>{ this->width(), 22.0f });
            m_button_panel->set_position(ds::point<f32>{
                this->width() - (m_button_panel->preferred_size().width + 5.0f), 3.0f });

            m_button_panel->perform_layout();
        }
    }

    void Dialog::refresh_relative_placement()
    {
        // helper to maintain nested window
        // position values, Popup overrides
    }
}
