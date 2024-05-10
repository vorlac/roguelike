#include <memory>
#include <ranges>
#include <tuple>
#include <utility>

#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "ds/rect.hpp"
#include "gfx/vg/nanovg_state.hpp"
#include "ui/canvas.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/popup.hpp"
#include "utils/conversions.hpp"
#include "utils/properties.hpp"
#include "widgets/scroll_dialog.hpp"

namespace rl::ui {
    Canvas::Canvas(MainWindow* main_window, const ds::rect<f32>& rect, const Mouse& mouse,
                   const Keyboard& kb, const std::unique_ptr<NVGRenderer>& nvg_renderer)
        : Widget{ nullptr, nvg_renderer }
        , m_mouse{ mouse }
        , m_keyboard{ kb }
        , m_main_window{ main_window }
    {
        this->set_rect(ds::rect{
            ds::point<f32>{ 0.0f, 0.0f },
            rect.size,
        });

        Widget::set_theme(new Theme{});

        m_last_interaction = m_timer.elapsed();
    }

    bool Canvas::update() const
    {
        for (const auto& update_widget_func : m_update_callbacks)
            update_widget_func();

        return !m_update_callbacks.empty();
    }

    bool Canvas::draw_setup() const
    {
        return true;
    }

    bool Canvas::draw_contents() const
    {
        return true;
    }

    bool Canvas::draw_widgets()
    {
        const auto context{ m_renderer->context() };
        constexpr static f32 PIXEL_RATIO{ 1.0f };
        nvg::begin_frame(context, m_rect.size.width, m_rect.size.height, PIXEL_RATIO);

        this->draw();

        // ================================================================
        // ================= DEBUG RENDERING ==============================
        // ================================================================

        this->draw_mouse_intersection(m_mouse.pos());

        // ================================================================
        // ================================================================
        // ================================================================

        const f32 elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed > m_tooltip_delay) {
            const Widget* widget{ this->find_widget(m_mouse.pos()) };
            if (widget != nullptr && !widget->tooltip().empty()) {
                const f32 tooltip_width{ m_theme->tooltip_width };
                ds::rect bounds{ ds::rect<f32>::zero() };
                ds::point<f32> pos{ widget->abs_position() +
                                    ds::point<f32>{ widget->width() / 2.0f,
                                                    widget->height() + 10.0f } };

                nvg::set_font_face(context, text::font::style::Sans);
                nvg::set_font_size(context, 20.0f);
                nvg::set_text_align(context, Align::HLeft | Align::VTop);
                nvg::text_line_height_(context, 1.125f);
                nvg::text_bounds(context, pos, widget->tooltip(), bounds);

                f32 horiz{ bounds.size.width / 2.0f };
                if (bounds.size.width / 2.0f > (tooltip_width / 2.0f)) {
                    nvg::set_text_align(context, Align::HCenter | Align::VTop);
                    bounds = nvg::text_box_bounds(context, pos, tooltip_width, widget->tooltip());
                    horiz = bounds.size.width / 2.0f;
                }

                f32 shift{ 0.0f };
                if (pos.x - bounds.size.width - 8.0f < 0.0f) {
                    // Keep tooltips on screen
                    shift = pos.x - bounds.size.width - 8.0f;
                    pos.x -= shift;
                    bounds.pt.x -= shift;
                }

                nvg::global_alpha(context,
                                  std::min(1.0f, 2.0f * (elapsed - m_tooltip_delay)) * 0.8f);

                nvg::begin_path(context);
                nvg::fill_color(context, rl::Colors::DarkererGrey);
                nvg::rounded_rect(context, bounds.pt.x - 4.0f - horiz, bounds.pt.y - 4.0f,
                                  bounds.size.width + 8.0f, bounds.size.height + 8.0f, 3.0f);

                const f32 px{ (bounds.size.width / 2.0f) - horiz + shift };

                nvg::move_to(context, px, bounds.pt.y - 10);
                nvg::line_to(context, px + 7, bounds.pt.y + 1);
                nvg::line_to(context, px - 7, bounds.pt.y + 1);
                nvg::fill(context);

                nvg::fill_color(context, rl::Colors::White);
                nvg::font_blur_(context, 0.0f);
                nvg::text_box(context, { pos.x - horiz, pos.y }, tooltip_width, widget->tooltip());
            }
        }

        nvg::end_frame(context);

        return true;
    }

    bool Canvas::redraw()
    {
        m_redraw = true;
        return true;
    }

    bool Canvas::draw_teardown() const
    {
        // moved to Window::render_end()
        return true;
    }

    bool Canvas::draw_all()
    {
        if (m_redraw) {
            m_redraw = false;
            this->update();
            this->draw_setup();
            this->draw_contents();
            this->draw_widgets();
            this->draw_teardown();
            return true;
        }

        return false;
    }

    void Canvas::set_visible(const bool visible)
    {
        if (visible != m_visible) {
            m_visible = visible;
            visible ? this->show() : this->hide();
        }
    }

    ds::dims<i32> Canvas::frame_buffer_size() const
    {
        // Return the framebuffer size (potentially larger than size() on high-DPI screens)
        // TODO: is ds::dims<i32> get_render_size() good equivalent?
        return m_framebuf_size;
    }

    const std::function<void(ds::dims<f32>)>& Canvas::resize_callback() const
    {
        return m_resize_callback;
    }

    void Canvas::set_resize_callback(const std::function<void(ds::dims<f32>)>& callback)
    {
        m_resize_callback = callback;
    }

    void Canvas::set_mouse_mode(const MouseMode mouse_mode)
    {
        m_mouse_mode = mouse_mode;
    }

    void Canvas::add_update_callback(const std::function<void()>& refresh_func)
    {
        m_update_callbacks.push_back(refresh_func);
    }

    ComponentFormat Canvas::component_format() const
    {
        // Return the component format underlying the screen
        debug_assert(false, "not implemented");
        return 0;
    }

    PixelFormat Canvas::pixel_format() const
    {
        // Return the pixel format underlying the screen
        debug_assert(false, "not implemented");
        return 0;
    }

    bool Canvas::has_depth_buffer() const
    {
        // Does the framebuffer have a depth buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool Canvas::has_stencil_buffer() const
    {
        // Does the framebuffer have a stencil buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool Canvas::has_float_buffer() const
    {
        // Does the framebuffer use a floating point representation
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    std::string Canvas::title() const
    {
        return m_title;
    }

    void Canvas::set_size(const ds::dims<f32> size)
    {
        debug_assert(
            m_main_window != nullptr,
            "canvas missing window reference");

        Widget::set_size(size);
        m_main_window->set_size(size);
    }

    void Canvas::set_min_size(const ds::dims<f32> size)
    {
        Widget::set_min_size(size);
        m_main_window->set_min_size(size);
    }

    void Canvas::set_max_size(const ds::dims<f32> size)
    {
        Widget::set_min_size(size);
        m_main_window->set_max_size(size);
    }

    bool Canvas::tooltip_fade_in_progress()
    {
        // Is a tooltip currently fading in?
        const f32 elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed < (m_tooltip_delay / 2.0f) || elapsed > (m_tooltip_delay * 2.0f))
            return false;

        // Temporarily increase the frame rate to fade in the tooltip
        const Widget* widget{ this->find_widget(m_mouse.pos()) };
        return widget != nullptr && !widget->tooltip().empty();
    }

    void Canvas::dispose_dialog(const ScrollableDialog* dialog)
    {
        const bool match_found{
            std::ranges::find_if(
                m_focus_path,
                [&](const Widget* w) {
                    return w == dialog;
                }) != m_focus_path.end()
        };

        if (match_found)
            m_focus_path.clear();
        if (m_active_dialog == dialog) {
            m_active_dialog = nullptr;
            m_active_widget = nullptr;
        }

        this->remove_child(dialog);
    }

    void Canvas::center_dialog(ScrollableDialog* dialog) const
    {
        if (dialog->size() == ds::dims<f32>::zero()) {
            const auto pref_size{ dialog->preferred_size() };
            dialog->set_size(pref_size);
            dialog->perform_layout();
        }

        const ds::dims<f32> offset{ ((m_rect.size - dialog->size()) / 2.0f) - m_rect.pt };
        const ds::point<f32> position{ offset.width, offset.height };
        dialog->set_position(position);
    }

    void Canvas::update_focus(Widget* widget)
    {
        for (const auto focus_widget : m_focus_path) {
            if (!focus_widget->focused())
                continue;

            focus_widget->on_focus_lost();
        }
        m_focus_path.clear();

        const ScrollableDialog* dialog{};
        while (widget != nullptr) {
            m_focus_path.push_back(widget);
            const ScrollableDialog* dlg{ dynamic_cast<ScrollableDialog*>(widget) };
            if (dlg != nullptr)
                dialog = dlg;

            widget = widget->parent();
        }

        for (const auto focus_widget : std::ranges::reverse_view{ m_focus_path })
            focus_widget->on_focus_gained();

        // if (dialog != nullptr)
        //     this->move_dialog_to_front(dialog);
    }

    void Canvas::move_dialog_to_front(ScrollableDialog* dialog)
    {
        const auto removal_iterator{ std::ranges::remove(m_children, dialog).begin() };
        m_children.erase(removal_iterator, m_children.end());
        m_children.push_back(dialog);

        bool changed{ true };
        while (changed) {
            changed = false;
            size_t base_idx{ 0 };
            for (size_t idx = 0; idx < m_children.size(); ++idx) {
                if (m_children[idx] == dialog)
                    base_idx = idx;
            }

            for (size_t idx = 0; idx < m_children.size(); ++idx) {
                const auto popup{ dynamic_cast<Popup*>(m_children[idx]) };
                if (popup != nullptr && popup->parent_dialog() == dialog && idx < base_idx) {
                    this->move_dialog_to_front(popup);
                    changed = true;
                    break;
                }
            }
        }
    }

    bool Canvas::on_moved(const ds::point<f32>& pt)
    {
        this->set_position(pt);
        return true;
    }

    bool Canvas::on_resized(const ds::dims<f32> size)
    {
        if (math::equal(size.area(), 0.0f))
            return false;

        this->set_size({
            size.width / m_pixel_ratio,
            size.height / m_pixel_ratio,
        });

        this->perform_layout();
        if (m_resize_callback != nullptr)
            m_resize_callback(m_rect.size);

        this->redraw();
        return true;
    }

    bool Canvas::on_mouse_move_event(const Mouse& mouse, const Keyboard& kb)
    {
        bool handled{ false };

        const auto& mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();
        if (m_mouse_mode != MouseMode::Ignore) {
            switch (m_mouse_mode) {
                case MouseMode::Drag:
                {
                    LocalTransform transform{ m_active_dialog };
                    handled |= m_active_dialog->on_mouse_drag(mouse, kb);
                    break;
                }
                case MouseMode::Resize:
                {
                    LocalTransform transform{ m_active_dialog };
                    handled |= m_active_dialog->on_mouse_drag(mouse, kb);
                    break;
                }
                case MouseMode::Propagate:
                {
                    m_active_dialog = nullptr;
                    Widget* widget{ this->find_widget(mouse_pos) };
                    if (widget != nullptr) {
                        ScrollableDialog* dialog{ dynamic_cast<ScrollableDialog*>(widget) };
                        if (dialog == widget) {
                            auto [mode, component, grab_pos] = dialog->check_interaction(mouse_pos);
                            // if the dialog is resizable and the mouse is at grab location
                            if ((mode & Interaction::Resize) != 0) {
                                handled |= false;
                            }
                            // if the dialog is moveable and the cursor is over the header
                            if ((mode & Interaction::Move) != 0) {
                                handled |= false;
                            }

                            m_active_dialog = dialog;
                            if (dialog->resizable() && grab_pos != Side::None)
                                mouse.set_cursor(grab_pos);
                            else if (dialog->cursor() != m_mouse.active_cursor())
                                m_mouse.set_cursor(dialog->cursor());
                        }
                        else if (widget->cursor() != m_mouse.active_cursor())
                            m_mouse.set_cursor(widget->cursor());
                    }
                    break;
                }

                case MouseMode::Ignore:
                    debug_assert("Unhandled/invalid Canvas mouse mode");
                    break;
            }
        }

        if (!handled)
            handled |= Widget::on_mouse_move(mouse, kb);

        m_redraw |= handled;
        return false;
    }

    bool Canvas::on_mouse_button_pressed_event(const Mouse& mouse, const Keyboard& kb)
    {
        m_active_dialog = nullptr;
        m_last_interaction = m_timer.elapsed();
        if (m_mouse_mode == MouseMode::Ignore)
            return true;

        const ds::point<f32> mouse_pos{ mouse.pos() };
        if (m_focus_path.size() > 1) {
            // Since Dialogs are always direct children of the Canvas and the tree is
            // represented where the root (Canvas) is the last item in the list, if a
            // ScrollableDialog is focused, then it will always be the 2nd to last item in the
            // m_focus_path vector.
            ScrollableDialog* dialog{ dynamic_cast<ScrollableDialog*>(
                m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr) {
                m_active_dialog = dialog;
                auto [mode, component, grab_pos] = dialog->check_interaction(mouse_pos);
                if (mode == Interaction::Modal && !dialog->contains(mouse_pos))
                    return false;
            }
        }

        switch (m_mouse_mode) {
            case MouseMode::Propagate:
            {
                m_active_widget = find_widget(mouse_pos);
                m_active_dialog = dynamic_cast<ScrollableDialog*>(m_active_widget);

                if (m_active_dialog != nullptr) {
                    const bool drag_btn_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
                    const bool resize_btn_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
                    if (resize_btn_pressed && m_active_dialog != nullptr) {
                        auto [mode, comp, grab_pos] = m_active_dialog->check_interaction(mouse_pos);
                        if (grab_pos != Side::None) {
                            m_mouse_mode = MouseMode::Resize;
                            m_redraw |= m_active_dialog->on_mouse_button_pressed(mouse, kb);
                        }
                    }

                    if (!m_redraw && drag_btn_pressed) {
                        m_mouse_mode = MouseMode::Drag;
                        m_redraw |= m_active_dialog->on_mouse_button_pressed(mouse, kb);
                    }
                }

                break;
            }
            case MouseMode::Drag:
                [[fallthrough]];
            case MouseMode::Resize:
                debug_assert("Invalid/unhandled canvas mouse mode");
                [[fallthrough]];
            case MouseMode::Ignore:
                break;
        }

        m_redraw |= Widget::on_mouse_button_pressed(mouse, kb);

        return false;
    }

    bool Canvas::on_mouse_button_released_event(const Mouse& mouse, const Keyboard& kb)
    {
        if (m_mouse_mode == MouseMode::Ignore)
            return true;

        const ds::point<f32> mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        if (m_focus_path.size() > 1) {
            const auto dialog{ dynamic_cast<ScrollableDialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->mode_active(Interaction::Modal)) {
                if (!dialog->contains(mouse_pos))
                    return true;
            }
        }

        const Widget* drop_widget{ this->find_widget(mouse_pos) };
        if (drop_widget != nullptr && drop_widget->cursor() != m_mouse.active_cursor())
            m_mouse.set_cursor(drop_widget->cursor());

        switch (m_mouse_mode) {
            case MouseMode::Drag:
            {
                debug_assert(m_active_dialog != nullptr,
                             "canvas in drag mode but no widgets active");

                m_mouse_mode = MouseMode::Propagate;
                if (drop_widget != m_active_dialog) {
                    LocalTransform transform{ m_active_dialog->parent() };
                    m_redraw |= m_active_dialog->on_mouse_button_released(mouse, kb);
                }

                const bool drag_btn_released{ mouse.is_button_released(Mouse::Button::Left) };
                if (drag_btn_released)
                    m_active_dialog = nullptr;

                break;
            }
            case MouseMode::Resize:
            {
                debug_assert(m_active_dialog != nullptr,
                             "canvas in resize mode but no widgets active");

                LocalTransform transform{ m_active_dialog->parent() };
                m_redraw |= m_active_dialog->on_mouse_button_released(mouse, kb);
                m_mouse_mode = MouseMode::Propagate;

                const bool resize_btn_released{ mouse.is_button_released(Mouse::Button::Left) };
                if (resize_btn_released)
                    m_active_dialog = nullptr;

                break;
            }

            default:
                debug_assert("Invalid/unhandled UI Canvas mouse mode");
                [[fallthrough]];
            case MouseMode::Ignore:
            case MouseMode::Propagate:
                break;
        }

        m_active_dialog = nullptr;
        m_mouse_mode = MouseMode::Propagate;

        m_redraw |= Widget::on_mouse_button_released(mouse, kb);
        return false;
    }

    bool Canvas::on_mouse_scroll_event(const Mouse& mouse, const Keyboard& kb)
    {
        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1) {
            auto dialog{ dynamic_cast<ScrollableDialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->mode_active(Interaction::Modal))
                if (!dialog->contains(mouse.pos()))
                    return true;
        }

        m_redraw |= Widget::on_mouse_scroll(mouse, kb);
        return false;
    }

    bool Canvas::on_key_pressed(const Keyboard& kb)
    {
        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_key_pressed(kb);
        return m_redraw;
    }

    bool Canvas::on_key_released(const Keyboard& kb)
    {
        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_key_released(kb);
        return m_redraw;
    }

    bool Canvas::on_character_input(const Keyboard& kb)
    {
        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_character_input(kb);
        return m_redraw;
    }
}
