#include <string>
#include <utility>

#include "core/ui/canvas.hpp"
#include "core/ui/layouts/box_layout.hpp"
#include "core/ui/widgets/dialog.hpp"
#include "ds/dims.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"

namespace rl::ui {

    Dialog::Dialog(Widget* parent, std::string title)
        : Widget{ parent }
        , m_title{ std::move(title) }
    {
        m_resizable = true;
        scoped_log();
    }

    const std::string& Dialog::title() const
    {
        scoped_log();
        return m_title;
    }

    void Dialog::set_title(const std::string& title)
    {
        scoped_log();
        m_title = title;
    }

    bool Dialog::modal() const
    {
        scoped_log();
        return m_modal;
    }

    void Dialog::set_modal(const bool modal)
    {
        scoped_log();
        m_modal = modal;
    }

    Widget* Dialog::button_panel()
    {
        scoped_log();
        if (m_button_panel == nullptr)
        {
            m_button_panel = new Widget{ this };
            m_button_panel->set_layout(new BoxLayout{
                Orientation::Horizontal,
                Alignment::Center,
                0.0f,
                4.0f,
            });
        }

        return m_button_panel;
    }

    void Dialog::dispose()
    {
        scoped_log();
        Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        dynamic_cast<Canvas*>(owner)->dispose_dialog(this);
    }

    void Dialog::center()
    {
        scoped_log();

        Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        dynamic_cast<Canvas*>(owner)->center_dialog(this);
    }

    f32 Dialog::header_height() const
    {
        scoped_logger(log_level::debug, "{}", m_title.empty() ? 0 : m_theme->dialog_header_height);

        if (!m_title.empty())
            return m_theme->dialog_header_height;

        return 0.0f;
    }

    void Dialog::draw()
    {
        scoped_trace(log_level::trace);

        auto&& context{ m_renderer->context() };
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

            if (!m_title.empty())
            {
                m_renderer->draw_path(false, [&] {
                    nvg::PaintStyle header_style{ nvg::linear_gradient(
                        context, m_rect.pt.x, m_rect.pt.y, m_rect.pt.x, m_rect.pt.y + header_height,
                        m_theme->dialog_header_gradient_top, m_theme->dialog_header_gradient_bot) };

                    m_renderer->draw_rounded_rect(
                        ds::rect<f32>{
                            m_rect.pt.x,
                            m_rect.pt.y,
                            m_rect.size.width,
                            header_height,
                        },
                        corner_radius);

                    m_renderer->fill_current_path(std::move(header_style));
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

                nvg::font_size(context, m_theme->tooltip_font_size);
                nvg::font_face(context, m_theme->tooltip_font_name.data());
                nvg::text_align(context, nvg::Align::HCenter | nvg::Align::VMiddle);

                // header text shadow
                nvg::font_blur(context, 2.0f);
                nvg::fill_color(context, m_theme->text_shadow);
                nvg::text(context, m_rect.pt.x + (m_rect.size.width / 2.0f),
                          m_rect.pt.y + (header_height / 2.0f), m_title.c_str());

                // Header text
                nvg::font_blur(context, 0.0f);
                nvg::fill_color(context, m_focused ? m_theme->dialog_title_focused
                                                   : m_theme->dialog_title_unfocused);
                nvg::text(context, m_rect.pt.x + (m_rect.size.width / 2.0f),
                          m_rect.pt.y + (header_height / 2.0f) - 1.0f, m_title.c_str());
            }
        });

        Widget::draw();
    }

    bool Dialog::on_mouse_entered(const Mouse& mouse)
    {
        scoped_log();
        Widget::on_mouse_entered(mouse);

        if (m_resizable)
        {
            const Side border_grab{ m_rect.edge_overlap(RESIZE_SELECT_BUFFER, mouse.pos()) };
            // TODO: debug - remove later
            if (m_resize_grab_point_side != border_grab)
                int [[maybe_unused]] breakhere = 123;

            m_resize_grab_point_side = border_grab;
        }

        return true;
    }

    bool Dialog::on_mouse_exited(const Mouse& mouse)
    {
        scoped_log();
        Widget::on_mouse_entered(mouse);

        if (m_resizable)
        {
            const Side border_grab{ m_rect.edge_overlap(RESIZE_SELECT_BUFFER, mouse.pos()) };
            // TODO: debug - remove later
            if (m_resize_grab_point_side != border_grab)
                int [[maybe_unused]] breakhere = 123;

            m_resize_grab_point_side = border_grab;
        }

        return true;
    }

    bool Dialog::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::debug, "pt:{}, rel:{}", mouse.pos(), mouse.pos_delta());
        if (m_drag_move && mouse.is_button_down(Mouse::Button::Left))
        {
            diag_log("m_pos_1={} mouse_delta={}", m_rect.pt, mouse.pos_delta());
            m_rect.pt += mouse.pos_delta();
            m_rect.pt.x = std::max(m_rect.pt.x, 0.0f);
            m_rect.pt.y = std::max(m_rect.pt.y, 0.0f);

            const ds::dims relative_size{ this->parent()->size() - m_rect.size };
            diag_log("m_pos_2={} rel_size={}", m_rect.pt, relative_size);
            m_rect.pt.x = std::min(m_rect.pt.x, relative_size.width);
            m_rect.pt.y = std::min(m_rect.pt.y, relative_size.height);

            diag_log("m_pos_3={}", m_rect.pt);
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_pressed());
        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        if (!m_drag_resize)
        {
            const Side border_grab{ m_rect.edge_overlap(RESIZE_SELECT_BUFFER, mouse.pos()) };
            // TODO: debug - remove later
            if (m_resize_grab_point_side != border_grab)
                int [[maybe_unused]] breakhere = 123;

            m_resize_grab_point_side = border_grab;
            m_drag_resize = m_resize_grab_point_side != Side::None;
        }

        if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            m_resize_grab_point_side = Side::None;
            const f32 offset_height{ mouse.pos().y - m_rect.pt.y };
            m_drag_move = offset_height < m_theme->dialog_header_height;
            diag_log("Dialog::m_drag_move={} offset_height={}", m_drag_move, offset_height);
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_released());
        if (Widget::on_mouse_button_released(mouse, kb))
            return true;

        if (mouse.is_button_released(Mouse::Button::Left))
        {
            m_drag_move = false;
            m_drag_resize = false;
            diag_log("Dialog::m_drag_move={}", m_drag_move);
            diag_log("Dialog::m_drag_resize={}", m_drag_move);
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::debug, "pos:{} wheel:{}", mouse.pos(), mouse.wheel());
        Widget::on_mouse_scroll(mouse, kb);
        return true;
    }

    ds::dims<f32> Dialog::preferred_size() const
    {
        scoped_trace(log_level::debug);

        if (m_button_panel != nullptr)
            m_button_panel->hide();
        const ds::dims result{ Widget::preferred_size() };
        if (m_button_panel != nullptr)
            m_button_panel->show();

        auto&& context{ m_renderer->context() };
        nvg::font_size(context, m_theme->dialog_title_font_size);
        nvg::font_face(context, m_theme->dialog_title_font_name.data());

        // [xmin, ymin, xmax, ymax]
        std::array<f32, 4> bounds{};
        nvg::text_bounds(context, 0, 0, m_title.c_str(), nullptr, bounds.data());

        constexpr static f32 TEXT_SIZE_WIDTH_PADDING{ 20.0f };
        ds::rect bounding_rect{
            ds::point{ 0.0f, 0.0f },
            ds::dims{
                std::max(result.width, bounds[2] - bounds[0] + TEXT_SIZE_WIDTH_PADDING),
                std::max(result.height, bounds[3] - bounds[1]),
            },
        };

        return bounding_rect.size;
    }

    void Dialog::perform_layout()
    {
        scoped_log();
        if (m_button_panel == nullptr)
            Widget::perform_layout();
        else
        {
            m_button_panel->hide();

            Widget::perform_layout();
            for (const auto bp_child : m_button_panel->children())
            {
                bp_child->set_fixed_size({
                    22.0f,
                    22.0f,
                });
                bp_child->set_font_size(15.0f);
            }

            m_button_panel->show();
            m_button_panel->set_size(ds::dims{
                this->width(),
                22.0f,
            });
            m_button_panel->set_position(ds::point{
                this->width() - (m_button_panel->preferred_size().width + 5.0f),
                3.0f,
            });

            m_button_panel->perform_layout();
        }
    }

    void Dialog::refresh_relative_placement()
    {
        // helper to maintain nested window
        // position values, Popup overrides
    }
}
