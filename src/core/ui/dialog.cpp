#include <string>
#include <utility>

#include "core/ui/canvas.hpp"
#include "core/ui/dialog.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"

namespace rl::ui {

    Dialog::Dialog(Widget* parent, std::string title)
        : Widget{ parent }
        , m_title{ std::move(title) }
    {
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
            m_button_panel = new Widget(this);
            m_button_panel->set_layout(
                new BoxLayout{ Orientation::Horizontal, Alignment::Center, 0.0f, 4.0f });
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
            nvg::begin_path(context);
            nvg::rounded_rect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
            nvg::fill_color(context, m_mouse_focus ? m_theme->dialog_fill_focused
                                                   : m_theme->dialog_fill_unfocused);
            nvg::fill(context);

            // Dialog shadow
            const nvg::PaintStyle shadow_paint{ nvg::box_gradient(
                context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius * 2.0f,
                drop_shadow_size * 2.0f, m_theme->dialog_shadow, m_theme->transparent) };

            m_renderer->scoped_draw([&] {
                nvg::reset_scissor(context);
                nvg::begin_path(context);
                nvg::rect(context, m_pos.x - drop_shadow_size, m_pos.y - drop_shadow_size,
                          m_size.width + 2.0f * drop_shadow_size,
                          m_size.height + 2.0f * drop_shadow_size);
                nvg::rounded_rect(context, m_pos.x, m_pos.y, m_size.width, m_size.height,
                                  corner_radius);
                nvg::path_winding(context, nvg::Solidity::NVGHole);
                nvg::fill_paint(context, shadow_paint);
                nvg::fill(context);
            });

            if (!m_title.empty())
            {
                const nvg::PaintStyle header_paint{ nvg::linear_gradient(
                    context, m_pos.x, m_pos.y, m_pos.x, m_pos.y + header_height,
                    m_theme->dialog_header_gradient_top, m_theme->dialog_header_gradient_bot) };

                nvg::begin_path(context);
                nvg::rounded_rect(context, m_pos.x, m_pos.y, m_size.width, header_height,
                                  corner_radius);

                nvg::fill_paint(context, header_paint);
                nvg::fill(context);

                nvg::begin_path(context);
                nvg::rounded_rect(context, m_pos.x, m_pos.y, m_size.width, header_height,
                                  corner_radius);
                nvg::stroke_color(context, m_theme->dialog_header_sep_top);

                nvg::save(context);
                nvg::intersect_scissor(context, m_pos.x, m_pos.y, m_size.width, 0.5f);
                nvg::stroke(context);
                nvg::restore(context);

                nvg::begin_path(context);
                nvg::move_to(context, m_pos.x + 0.5f, m_pos.y + header_height - 1.5f);
                nvg::line_to(context, m_pos.x + m_size.width - 0.5f, m_pos.y + header_height - 1.5f);
                nvg::stroke_color(context, m_theme->dialog_header_sep_bot);
                nvg::stroke(context);

                nvg::font_size(context, m_theme->tooltip_font_size);
                nvg::font_face(context, m_theme->tooltip_font_name.data());
                nvg::text_align(context, nvg::Align::NVGAlignCenter | nvg::Align::NVGAlignMiddle);

                // header text shadow
                nvg::font_blur(context, 2.0f);
                nvg::fill_color(context, m_theme->text_shadow);
                nvg::text(context, m_pos.x + (m_size.width / 2.0f),
                          m_pos.y + (header_height / 2.0f), m_title.c_str());

                // Header text
                nvg::font_blur(context, 0.0f);
                nvg::fill_color(context, m_focused ? m_theme->dialog_title_focused
                                                   : m_theme->dialog_title_unfocused);
                nvg::text(context, m_pos.x + (m_size.width / 2.0f),
                          m_pos.y + (header_height / 2.0f) - 1.0f, m_title.c_str());
            }
        });

        Widget::draw();
    }

    bool Dialog::on_mouse_entered(const Mouse& mouse)
    {
        scoped_log();
        Widget::on_mouse_entered(mouse);
        return true;
    }

    bool Dialog::on_mouse_exited(const Mouse& mouse)
    {
        scoped_log();
        Widget::on_mouse_exited(mouse);
        return true;
    }

    bool Dialog::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::debug, "pt:{}, rel:{}", mouse.pos(), mouse.pos_delta());
        if (m_drag && mouse.is_button_held(Mouse::Button::Left))
        {
            diag_log("m_pos_1={} mouse_delta={}", m_pos, mouse.pos_delta());
            m_pos += mouse.pos_delta();
            m_pos.x = std::max(m_pos.x, 0.0f);
            m_pos.y = std::max(m_pos.y, 0.0f);

            const ds::dims relative_size{ this->parent()->size() - m_size };
            diag_log("m_pos_2={} rel_size={}", m_pos, relative_size);
            m_pos.x = std::min(m_pos.x, relative_size.width);
            m_pos.y = std::min(m_pos.y, relative_size.height);

            diag_log("m_pos_3={}", m_pos);
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_pressed());
        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            const f32 offset_height{ mouse.pos().y - m_pos.y };
            m_drag = offset_height < m_theme->dialog_header_height;
            diag_log("Dialog::m_drag={} offset_height={}", m_drag, offset_height);
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
            m_drag = false;
            diag_log("Dialog::m_drag={}", m_drag);
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

        auto&& context{ m_renderer->context() };
        const ds::dims result{ Widget::preferred_size() };

        if (m_button_panel != nullptr)
            m_button_panel->show();

        nvg::font_size(context, m_theme->dialog_title_font_size);
        nvg::font_face(context, m_theme->dialog_title_font_name.data());

        // [xmin, ymin, xmax, ymax]
        std::array<f32, 4> bounds = {};
        nvg::text_bounds(context, 0, 0, m_title.c_str(), nullptr, bounds.data());

        constexpr static f32 TEXT_SIZE_WIDTH_PADDING{ 20.0f };
        ds::rect bounding_rect{
            ds::point<f32>::zero(),
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
            for (auto bp_child : m_button_panel->children())
            {
                bp_child->set_fixed_size({ 22.0f, 22.0f });
                bp_child->set_font_size(15.0f);
            }

            m_button_panel->show();

            m_button_panel->set_size(ds::dims{ this->width(), 22.0f });
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
