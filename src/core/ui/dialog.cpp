#include <string>
#include <utility>

#include "core/ui/canvas.hpp"
#include "core/ui/dialog.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"

namespace rl::ui {

    Dialog::Dialog(Widget* parent, const std::string& title)
        : Widget{ parent }
        , m_title{ title }
        , m_button_panel{ nullptr }
        , m_modal{ false }
        , m_drag{ false }
    {
    }

    const std::string& Dialog::title() const
    {
        return m_title;
    }

    void Dialog::set_title(const std::string& title)
    {
        m_title = title;
    }

    bool Dialog::modal() const
    {
        return m_modal;
    }

    void Dialog::set_modal(bool modal)
    {
        m_modal = modal;
    }

    Widget* Dialog::button_panel()
    {
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
        Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        static_cast<UICanvas*>(owner)->dispose_dialog(this);
    }

    void Dialog::center()
    {
        Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        static_cast<UICanvas*>(owner)->center_dialog(this);
    }

    f32 Dialog::header_height() const
    {
        if (!m_title.empty())
            return m_theme->dialog_header_height;

        return 0.0f;
    }

    void Dialog::draw()
    {
        auto&& context{ m_renderer->context() };
        const f32 drop_shadow_size{ m_theme->dialog_drop_shadow_size };
        const f32 corner_radius{ m_theme->dialog_corner_radius };
        const f32 header_height{ this->header_height() };

        nvg::Save(context);
        nvg::BeginPath(context);
        nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        nvg::FillColor(context, m_mouse_focus ? m_theme->dialog_fill_focused.nvg()
                                              : m_theme->dialog_fill_unfocused.nvg());
        nvg::Fill(context);

        // Dialog shadow
        nvg::NVGpaint shadow_paint = nvg::BoxGradient(
            context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius * 2.0f,
            drop_shadow_size * 2.0f, m_theme->dialog_shadow.nvg(), m_theme->transparent.nvg());

        nvg::Save(context);
        nvg::ResetScissor(context);
        nvg::BeginPath(context);
        nvg::Rect(context, m_pos.x - drop_shadow_size, m_pos.y - drop_shadow_size,
                  m_size.width + 2.0f * drop_shadow_size, m_size.height + 2.0f * drop_shadow_size);
        nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        nvg::PathWinding(context, nvg::NVGsolidity::NVG_HOLE);
        nvg::FillPaint(context, shadow_paint);
        nvg::Fill(context);
        nvg::Restore(context);

        if (!m_title.empty())
        {
            nvg::NVGpaint header_paint{ nvg::LinearGradient(
                context, m_pos.x, m_pos.y, m_pos.x, m_pos.y + header_height,
                m_theme->dialog_header_gradient_top.nvg(),
                m_theme->dialog_header_gradient_bot.nvg()) };

            nvg::BeginPath(context);
            nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, header_height, corner_radius);

            nvg::FillPaint(context, header_paint);
            nvg::Fill(context);

            nvg::BeginPath(context);
            nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, header_height, corner_radius);
            nvg::StrokeColor(context, m_theme->dialog_header_sep_top.nvg());

            nvg::Save(context);
            nvg::IntersectScissor(context, m_pos.x, m_pos.y, m_size.width, 0.5f);
            nvg::Stroke(context);
            nvg::Restore(context);

            nvg::BeginPath(context);
            nvg::MoveTo(context, m_pos.x + 0.5f, m_pos.y + header_height - 1.5f);
            nvg::LineTo(context, m_pos.x + m_size.width - 0.5f, m_pos.y + header_height - 1.5f);
            nvg::StrokeColor(context, m_theme->dialog_header_sep_bot.nvg());
            nvg::Stroke(context);

            nvg::FontSize(context, m_theme->tooltip_font_size);
            nvg::FontFace(context, m_theme->tooltip_font_name.data());
            nvg::TextAlign(context, Text::Alignment::HCenterVMiddle);

            // header text shadow
            nvg::FontBlur(context, 0.0f);
            nvg::FillColor(context, m_theme->text_shadow.nvg());
            nvg::Text(context, m_pos.x + (m_size.width / 2.0f), m_pos.y + (header_height / 2.0f),
                      m_title.c_str());

            // Header text
            nvg::FontBlur(context, 0.0f);
            nvg::FillColor(context, m_focused ? m_theme->dialog_title_focused.nvg()
                                              : m_theme->dialog_title_unfocused.nvg());
            nvg::Text(context, m_pos.x + m_size.width / 2.0f, m_pos.y + header_height / 2.0f - 1.0f,
                      m_title.c_str());
        }

        nvg::Restore(context);
        Widget::draw();
    }

    bool Dialog::on_mouse_entered(const Mouse& mouse)
    {
        Widget::on_mouse_entered(mouse);
        return true;
    }

    bool Dialog::on_mouse_exited(const Mouse& mouse)
    {
        Widget::on_mouse_exited(mouse);
        return true;
    }

    bool Dialog::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            log::info("Dialog::on_mouse_drag [pt:{}, rel:{}, btn:{}, mod:{}]", mouse.pos(),
                      mouse.pos_delta(), std::to_underlying(mouse.button_pressed()),
                      kb.is_button_down(Keyboard::Scancode::Modifiers));

        if (m_drag && mouse.is_button_held(Mouse::Button::Left))
        {
            m_pos += mouse.pos_delta();

            m_pos.x = std::max(m_pos.x, 0.0f);
            m_pos.y = std::max(m_pos.y, 0.0f);

            const ds::dims<f32> relative_size{ this->parent()->size() - m_size };

            m_pos.x = std::min(m_pos.x, relative_size.width);
            m_pos.y = std::min(m_pos.y, relative_size.height);

            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("Dialog::on_mouse_button_pressed [button:{}]",
                          std::to_underlying(mouse.button_pressed()));

        if (Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            f32 offset_height{ mouse.pos().y - m_pos.y };
            m_drag = offset_height < m_theme->dialog_header_height;
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("Dialog::on_mouse_button_released [button:{}]",
                          std::to_underlying(mouse.button_released()));

        if (Widget::on_mouse_button_released(mouse, kb))
            return true;

        if (mouse.is_button_released(Mouse::Button::Left))
        {
            m_drag = false;
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("Dialog::on_mouse_scroll [pos:{}, rel:{}]", mouse.pos(), mouse.wheel());

        return Widget::on_mouse_scroll(mouse, kb);
    }

    ds::dims<f32> Dialog::preferred_size() const
    {
        if (m_button_panel != nullptr)
            m_button_panel->hide();

        auto&& context{ m_renderer->context() };
        ds::dims<f32> result{ Widget::preferred_size() };

        if (m_button_panel != nullptr)
            m_button_panel->show();

        nvg::FontSize(context, 18.0f);
        nvg::FontFace(context, font::name::sans);

        std::array<f32, 4> bounds = {};
        nvg::TextBounds(context, 0, 0, m_title.c_str(), nullptr, bounds.data());

        return ds::dims<f32>(std::max(result.width, bounds[2] - bounds[0] + 20),
                             std::max(result.height, bounds[3] - bounds[1]));
    }

    void Dialog::perform_layout()
    {
        if (m_button_panel == nullptr)
            Widget::perform_layout();
        else
        {
            m_button_panel->set_visible(false);
            Widget::perform_layout();

            for (auto& bp_child : m_button_panel->children())
            {
                bp_child->set_fixed_size({ 22.0f, 22.0f });
                bp_child->set_font_size(15.0f);
            }

            m_button_panel->set_visible(true);
            m_button_panel->set_size({ this->width(), 22.0f });
            m_button_panel->set_position({
                this->width() - (m_button_panel->preferred_size().width + 5.0f),
                3.0f,
            });

            m_button_panel->perform_layout();
        }
    }

    void Dialog::refresh_relative_placement()
    {
        // helper to maintain nested window position values,
        // overridden in Popup
        return;
    }

}
