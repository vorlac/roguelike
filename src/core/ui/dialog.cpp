#include <string>
#include <utility>

#include "core/ui/dialog.hpp"
#include "core/ui/gui_canvas.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"
#include "graphics/vg/nanovg.hpp"
#include "utils/io.hpp"

namespace rl::ui {
    using namespace vg;

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
                new ui::BoxLayout{ Orientation::Horizontal, Alignment::Center, 0.0f, 4.0f });
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

    void Dialog::draw(NVGcontext* nvg_context)
    {
        const f32 drop_shadow_size{ m_theme->m_window_drop_shadow_size };
        const f32 corner_radius{ m_theme->m_window_corner_radius };
        const f32 header_height{ m_theme->m_window_header_height };

        // nvgSave(nvg_context);
        nvgBeginPath(nvg_context);
        nvgRoundedRect(nvg_context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        nvgFillColor(nvg_context, m_mouse_focus ? m_theme->m_window_fill_focused
                                                : m_theme->m_window_fill_unfocused);
        nvgFill(nvg_context);

        NVGpaint shadow_paint = nvgBoxGradient(
            nvg_context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius * 2.0f,
            drop_shadow_size * 2, m_theme->m_drop_shadow, m_theme->m_transparent);

        // nvgSave(nvg_context);
        nvgResetScissor(nvg_context);
        nvgBeginPath(nvg_context);
        nvgRect(nvg_context, m_pos.x - drop_shadow_size, m_pos.y - drop_shadow_size,
                m_size.width + 2 * drop_shadow_size, m_size.height + 2.0f * drop_shadow_size);
        nvgRoundedRect(nvg_context, m_pos.x, m_pos.y, m_size.width, m_size.height, corner_radius);
        nvgPathWinding(nvg_context, NVGsolidity::NVG_HOLE);
        nvgFillPaint(nvg_context, shadow_paint);
        nvgFill(nvg_context);
        // nvgRestore(nvg_context);

        if (!m_title.empty())
        {
            NVGpaint header_paint{ nvgLinearGradient(
                nvg_context, m_pos.x, m_pos.y, m_pos.x, m_pos.y + header_height,
                m_theme->m_window_header_gradient_top, m_theme->m_window_header_gradient_bot) };

            nvgBeginPath(nvg_context);
            nvgRoundedRect(nvg_context, m_pos.x, m_pos.y, m_size.width, header_height,
                           corner_radius);

            nvgFillPaint(nvg_context, header_paint);
            nvgFill(nvg_context);

            nvgBeginPath(nvg_context);
            nvgRoundedRect(nvg_context, m_pos.x, m_pos.y, m_size.width, header_height,
                           corner_radius);
            nvgStrokeColor(nvg_context, m_theme->m_window_header_sep_top);

            // nvgSave(nvg_context);
            // nvgIntersectScissor(nvg_context, m_pos.x, m_pos.y, m_size.width, 0.5f);
            nvgStroke(nvg_context);
            // nvgRestore(nvg_context);

            nvgBeginPath(nvg_context);
            nvgMoveTo(nvg_context, m_pos.x + 0.5f, m_pos.y + header_height - 1.5f);
            nvgLineTo(nvg_context, m_pos.x + m_size.width - 0.5f, m_pos.y + header_height - 1.5f);
            nvgStrokeColor(nvg_context, m_theme->m_window_header_sep_bot);
            nvgStroke(nvg_context);

            nvgFontSize(nvg_context, 18.0f);
            nvgFontFace(nvg_context, ui::font::name::sans_bold);
            nvgTextAlign(nvg_context, Text::Alignment::Centered);

            nvgFontBlur(nvg_context, 2.0f);
            nvgFillColor(nvg_context, m_theme->m_drop_shadow);
            nvgText(nvg_context, m_pos.x + m_size.width / 2.0f, m_pos.y + header_height / 2.0f,
                    m_title.c_str(), nullptr);

            nvgFontBlur(nvg_context, 0.0f);
            nvgFillColor(nvg_context, m_focused ? m_theme->m_window_title_focused
                                                : m_theme->m_window_title_unfocused);
            nvgText(nvg_context, m_pos.x + m_size.width / 2.0f,
                    m_pos.y + header_height / 2.0f - 1.0f, m_title.c_str(), nullptr);
        }

        // nvgRestore(nvg_context);

        ui::Widget::draw(nvg_context);
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

        if (ui::Widget::on_mouse_button_pressed(mouse, kb))
            return true;

        if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            f32 offset_height{ mouse.pos().y - m_pos.y };
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

    ds::dims<f32> Dialog::preferred_size(NVGcontext* nvg_context) const
    {
        if (m_button_panel != nullptr)
            m_button_panel->hide();

        ds::dims<f32> result{ ui::Widget::preferred_size(nvg_context) };

        if (m_button_panel != nullptr)
            m_button_panel->show();

        nvgFontSize(nvg_context, 18.0f);
        nvgFontFace(nvg_context, ui::font::name::sans_bold);

        std::array<f32, 4> bounds = {};
        nvgTextBounds(nvg_context, 0, 0, m_title.c_str(), nullptr, bounds.data());

        return ds::dims<f32>(std::max(result.width, bounds[2] - bounds[0] + 20),
                             std::max(result.height, bounds[3] - bounds[1]));
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
