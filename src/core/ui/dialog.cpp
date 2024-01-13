#include <string>
#include <utility>

#include <nanovg.h>

#include "core/ui/dialog.hpp"
#include "core/ui/gui_canvas.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"
#include "utils/io.hpp"

namespace rl::ui {

    Dialog::Dialog(ui::Widget* parent, const std::string& title)
        : ui::Widget{ parent }
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

    ui::Widget* Dialog::button_panel()
    {
        if (m_button_panel == nullptr)
        {
            m_button_panel = new ui::Widget(this);
            m_button_panel->set_layout(
                new ui::BoxLayout{ Orientation::Horizontal, Alignment::Center, 0, 4 });
        }

        return m_button_panel;
    }

    void Dialog::dispose()
    {
        ui::Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        static_cast<ui::UICanvas*>(owner)->dispose_dialog(this);
    }

    void Dialog::center()
    {
        ui::Widget* owner{ this };
        while (owner->parent() != nullptr)
            owner = owner->parent();

        static_cast<ui::UICanvas*>(owner)->center_dialog(this);
    }

    void Dialog::draw(NVGcontext* ctx)
    {
        const i32 ds{ m_theme->m_window_drop_shadow_size };
        const i32 cr{ m_theme->m_window_corner_radius };
        const i32 hh{ m_theme->m_window_header_height };

        // Draw window
        nvgSave(ctx);
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);
        nvgFillColor(ctx, m_mouse_focus ? m_theme->m_window_fill_focused
                                        : m_theme->m_window_fill_unfocused);
        nvgFill(ctx);

        // Draw a drop shadow
        NVGpaint shadow_paint{ nvgBoxGradient(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height,
                                              cr * 2, ds * 2, m_theme->m_drop_shadow,
                                              m_theme->m_transparent) };

        nvgSave(ctx);
        nvgResetScissor(ctx);
        nvgBeginPath(ctx);
        nvgRect(ctx, m_pos.x - ds, m_pos.y - ds, m_size.width + 2 * ds, m_size.height + 2 * ds);
        nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadow_paint);
        nvgFill(ctx);
        nvgRestore(ctx);

        if (!m_title.empty())
        {
            // Draw header
            NVGpaint header_paint{ nvgLinearGradient(ctx, m_pos.x, m_pos.y, m_pos.x, m_pos.y + hh,
                                                     m_theme->m_window_header_gradient_top,
                                                     m_theme->m_window_header_gradient_bot) };

            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, hh, cr);

            nvgFillPaint(ctx, header_paint);
            nvgFill(ctx);

            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, hh, cr);
            nvgStrokeColor(ctx, m_theme->m_window_header_sep_top);

            nvgSave(ctx);
            nvgIntersectScissor(ctx, m_pos.x, m_pos.y, m_size.width, 0.5f);
            nvgStroke(ctx);
            nvgRestore(ctx);

            nvgBeginPath(ctx);
            nvgMoveTo(ctx, m_pos.x + 0.5f, m_pos.y + hh - 1.5f);
            nvgLineTo(ctx, m_pos.x + m_size.width - 0.5f, m_pos.y + hh - 1.5f);
            nvgStrokeColor(ctx, m_theme->m_window_header_sep_bot);
            nvgStroke(ctx);

            nvgFontSize(ctx, 18.0f);
            nvgFontFace(ctx, ui::font::name::sans_bold);
            nvgTextAlign(ctx, NVGalign::NVG_ALIGN_CENTER | NVGalign::NVG_ALIGN_MIDDLE);

            nvgFontBlur(ctx, 2.0f);
            nvgFillColor(ctx, m_theme->m_drop_shadow);
            nvgText(ctx, m_pos.x + m_size.width / 2.0f, m_pos.y + hh / 2.0f, m_title.c_str(),
                    nullptr);

            nvgFontBlur(ctx, 0.0f);
            nvgFillColor(ctx, m_focused ? m_theme->m_window_title_focused
                                        : m_theme->m_window_title_unfocused);
            nvgText(ctx, m_pos.x + m_size.width / 2.0f, m_pos.y + hh / 2.0f - 1, m_title.c_str(),
                    nullptr);
        }

        nvgRestore(ctx);

        ui::Widget::draw(ctx);
    }

    bool Dialog::on_mouse_entered(const Mouse& mouse)
    {
        ui::Widget::on_mouse_entered(mouse);
        return true;
    }

    bool Dialog::on_mouse_exited(const Mouse& mouse)
    {
        ui::Widget::on_mouse_exited(mouse);
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

            m_pos.x = std::max(m_pos.x, 0);
            m_pos.y = std::max(m_pos.y, 0);

            const ds::dims<i32> relative_size{ this->parent()->size() - m_size };

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

        if (ui::Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            i32 offset_height{ mouse.pos().y - m_pos.y };
            m_drag = offset_height < m_theme->m_window_header_height;
            return true;
        }

        return false;
    }

    bool Dialog::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("Dialog::on_mouse_button_released [button:{}]",
                          std::to_underlying(mouse.button_released()));

        if (ui::Widget::on_mouse_button_released(mouse, kb))
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

        return ui::Widget::on_mouse_scroll(mouse, kb);
    }

    ds::dims<i32> Dialog::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_button_panel != nullptr)
            m_button_panel->hide();

        ds::dims<i32> result{ ui::Widget::preferred_size(nvg_context) };

        if (m_button_panel != nullptr)
            m_button_panel->show();

        nvgFontSize(nvg_context, 18.0f);
        nvgFontFace(nvg_context, ui::font::name::sans_bold);

        std::array<f32, 4> bounds = {};
        nvgTextBounds(nvg_context, 0, 0, m_title.c_str(), nullptr, bounds.data());

        return ds::dims<i32>(std::max(result.width, static_cast<i32>(bounds[2] - bounds[0] + 20)),
                             std::max(result.height, static_cast<i32>(bounds[3] - bounds[1])));
    }

    void Dialog::perform_layout(NVGcontext* ctx)
    {
        if (m_button_panel == nullptr)
            ui::Widget::perform_layout(ctx);
        else
        {
            m_button_panel->set_visible(false);
            ui::Widget::perform_layout(ctx);

            for (auto w : m_button_panel->children())
            {
                w->set_fixed_size({ 22, 22 });
                w->set_font_size(15);
            }

            m_button_panel->set_visible(true);
            m_button_panel->set_size({ this->width(), 22 });
            m_button_panel->set_position({
                this->width() - (m_button_panel->preferred_size(ctx).width + 5),
                3,
            });

            m_button_panel->perform_layout(ctx);
        }
    }

    void Dialog::refresh_relative_placement()
    {
        // helper to maintain nested window position values,
        // overridden in ui::Popup
        return;
    }

}
