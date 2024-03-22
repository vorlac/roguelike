#include <tuple>
#include <utility>

#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/widgets/popup.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/conversions.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/properties.hpp"
#include "widgets/scroll_dialog.hpp"

namespace rl::ui {

    Canvas::Canvas(const ds::rect<f32>& rect, const Mouse& mouse, const Keyboard& kb,
                   const std::unique_ptr<NVGRenderer>& nvg_renderer)
        : Widget{ nullptr, nvg_renderer }
        , m_mouse{ mouse }
        , m_keyboard{ kb }
    {
        m_rect.pt = { 0.0f, 0.0f };
        m_rect.size = rect.size;

        Widget::set_theme(new Theme{ nvg_renderer->context() });
        this->set_visible(true);

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
        this->update();
        return true;
    }

    bool Canvas::draw_widgets()
    {
        auto&& context{ m_renderer->context() };
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
        if (elapsed > m_tooltip_delay)
        {
            const Widget* widget{ this->find_widget(m_mouse.pos()) };
            if (widget != nullptr && !widget->tooltip().empty())
            {
                constexpr f32 tooltip_width{ 150.0f };
                std::array<f32, 4> bounds = { 0.0f };
                ds::point pos{
                    widget->position() +
                        ds::point{
                            (widget->width() / 2.0f),
                            widget->height() + 10.0f,
                        },
                };

                nvg::font_face(context, Font::Name::Sans);
                nvg::font_size(context, 20.0f);
                nvg::text_align(context, nvg::Align::HLeft | nvg::Align::VTop);
                nvg::text_line_height(context, 1.125f);
                nvg::text_bounds(context, pos.x, pos.y, widget->tooltip().c_str(), nullptr,
                                 bounds.data());

                f32 height{ (bounds[2] - bounds[0]) / 2.0f };
                if (height > (tooltip_width / 2.0f))
                {
                    nvg::text_align(context, nvg::Align::HCenter | nvg::Align::VTop);
                    nvg::text_box_bounds(context, pos.x, pos.y, tooltip_width,
                                         widget->tooltip().c_str(), nullptr, bounds.data());

                    height = (bounds[2] - bounds[0]) / 2;
                }

                f32 shift{ 0.0f };
                if (pos.x - height - 8.0f < 0.0f)
                {
                    // Keep tooltips on screen
                    shift = pos.x - height - 8.0f;
                    pos.x -= shift;
                    bounds[0] -= shift;
                    bounds[2] -= shift;
                }

                nvg::global_alpha(context,
                                  std::min(1.0f, 2.0f * (elapsed - m_tooltip_delay)) * 0.8f);

                nvg::begin_path(context);
                nvg::fill_color(context, rl::Colors::DarkererGrey);
                nvg::rounded_rect(context, bounds[0] - 4.0f - height, bounds[1] - 4.0f,
                                  (bounds[2] - bounds[0]) + 8.0f, (bounds[3] - bounds[1]) + 8.0f,
                                  3.0f);

                const f32 px{ ((bounds[2] + bounds[0]) / 2.0f) - height + shift };

                nvg::move_to(context, px, bounds[1] - 10);
                nvg::line_to(context, px + 7, bounds[1] + 1);
                nvg::line_to(context, px - 7, bounds[1] + 1);
                nvg::fill(context);

                nvg::fill_color(context, rl::Colors::White);
                nvg::font_blur(context, 0.0f);
                nvg::text_box(context, pos.x - height, pos.y, tooltip_width,
                              widget->tooltip().c_str(), nullptr);
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
        if (m_redraw)
        {
            m_redraw = false;
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
        if (visible != m_visible)
        {
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
        runtime_assert(false, "not implemented");
        return 0;
    }

    PixelFormat Canvas::pixel_format() const
    {
        // Return the pixel format underlying the screen
        runtime_assert(false, "not implemented");
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

    void Canvas::dispose_dialog(ScrollableDialog* dialog)
    {
        const bool match_found{ std::ranges::find_if(m_focus_path, [&](const Widget* w) {
                                    return w == dialog;
                                }) != m_focus_path.end() };
        if (match_found)
            m_focus_path.clear();

        if (m_active_dialog == static_cast<Widget*>(dialog))
        {
            m_active_dialog = nullptr;
            m_active_widget = nullptr;
        }

        this->remove_child(dialog);
    }

    void Canvas::center_dialog(ScrollableDialog* dialog) const
    {
        if (dialog->size() == ds::dims<f32>::zero())
        {
            auto pref_size{ dialog->preferred_size() };
            dialog->set_size(std::move(pref_size));
            dialog->perform_layout();
        }

        const ds::dims offset{ (((m_rect.size - dialog->size()) / 2.0f) - m_rect.pt) };
        ds::point position{ offset.width, offset.height };
        dialog->set_position(std::move(position));
    }

    void Canvas::update_focus(Widget* widget)
    {
        for (const auto focus_widget : m_focus_path)
        {
            if (!focus_widget->focused())
                continue;

            focus_widget->on_focus_lost();
        }
        m_focus_path.clear();

        ScrollableDialog* dialog{ nullptr };
        while (widget != nullptr)
        {
            m_focus_path.push_back(widget);
            ScrollableDialog* dlg{ dynamic_cast<ScrollableDialog*>(widget) };
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
        while (changed)
        {
            changed = false;
            size_t base_idx{ 0 };
            for (size_t idx = 0; idx < m_children.size(); ++idx)
                if (m_children[idx] == dialog)
                    base_idx = idx;

            for (size_t idx = 0; idx < m_children.size(); ++idx)
            {
                const auto popup{ dynamic_cast<Popup*>(m_children[idx]) };
                if (popup != nullptr && popup->parent_dialog() == dialog && idx < base_idx)
                {
                    this->move_dialog_to_front(popup);
                    changed = true;
                    break;
                }
            }
        }
    }

    bool Canvas::on_moved(ds::point<f32>&& pt)
    {
        scoped_log("{} => {}", m_rect, ds::rect{ pt, m_rect.size });
        this->set_position(std::forward<decltype(pt)>(pt));
        return true;
    }

    bool Canvas::on_resized(ds::dims<f32>&& size)
    {
        scoped_log("{} => {}", ds::rect{ m_rect.pt, m_rect.size },
                   ds::rect{ m_rect.pt, size / m_pixel_ratio });

        if (size.area() == 0.0f)
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

        auto mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();
        if (m_mouse_mode != MouseMode::Ignore)
        {
            switch (m_mouse_mode)
            {
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
                    if (widget != nullptr)
                    {
                        ScrollableDialog* dialog{ dynamic_cast<ScrollableDialog*>(widget) };
                        if (dialog == widget)
                        {
                            auto [mode, component, grab_pos] = dialog->check_interaction(mouse_pos);
                            // if the dialog is resizable and the mouse is at grab location
                            if ((mode & Interaction::Resize) != 0)
                            {
                                handled |= false;
                            }
                            // if the dialog is moveable and the cursor is over the header
                            if ((mode & Interaction::Move) != 0)
                            {
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
                    [[fallthrough]];
                default:
                    assert_msg("Unhandled/invalid Canvas mouse mode");
                    break;
            }

            scoped_logger(log_level::trace, "move_pos={}", mouse.pos());
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

        const ds::point mouse_pos{ mouse.pos() };
        if (m_focus_path.size() > 1)
        {
            // Since Dialogs are always direct children of the Canvas and the tree is represented
            // where the root (Canvas) is the last item in the list, if a ScrollableDialog is
            // focused, then it will always be the 2nd to last item in the m_focus_path vector.
            ScrollableDialog* dialog{ dynamic_cast<ScrollableDialog*>(
                m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr)
            {
                m_active_dialog = dialog;
                auto [mode, component, grab_pos] = dialog->check_interaction(mouse_pos);
                if (mode == Interaction::Modal && !dialog->contains(mouse_pos))
                    return false;
            }
        }

        switch (m_mouse_mode)
        {
            case MouseMode::Propagate:
            {
                m_active_widget = find_widget(mouse_pos);
                m_active_dialog = dynamic_cast<ScrollableDialog*>(m_active_widget);

                if (m_active_dialog != nullptr)
                {
                    bool drag_btn_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
                    bool resize_btn_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };

                    if (resize_btn_pressed && m_active_dialog != nullptr)
                    {
                        auto [mode, comp, grab_pos] = m_active_dialog->check_interaction(mouse_pos);
                        if (grab_pos != Side::None)
                        {
                            m_mouse_mode = MouseMode::Resize;
                            m_redraw |= m_active_dialog->on_mouse_button_pressed(mouse, kb);
                        }
                    }

                    if (!m_redraw && drag_btn_pressed)
                    {
                        m_mouse_mode = MouseMode::Drag;
                        m_redraw |= m_active_dialog->on_mouse_button_pressed(mouse, kb);
                    }
                }

                break;
            }
            case MouseMode::Drag:
                [[fallthrough]];
            case MouseMode::Resize:
                [[fallthrough]];
            default:
                assert_msg("Invalid/unhandled canvas mouse mode");
                [[fallthrough]];
            case MouseMode::Ignore:
                break;
        }

        m_redraw |= Widget::on_mouse_button_pressed(mouse, kb);

        return false;
    }

    bool Canvas::on_mouse_button_released_event(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_released());
        if (m_mouse_mode == MouseMode::Ignore)
            return true;

        const ds::point mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        if (m_focus_path.size() > 1)
        {
            auto dialog{ dynamic_cast<ScrollableDialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->mode_active(Interaction::Modal))
            {
                if (!dialog->contains(mouse_pos))
                    return true;
            }
        }

        Widget* drop_widget{ this->find_widget(mouse_pos) };
        if (drop_widget != nullptr && drop_widget->cursor() != m_mouse.active_cursor())
            m_mouse.set_cursor(drop_widget->cursor());

        switch (m_mouse_mode)
        {
            case MouseMode::Drag:
            {
                runtime_assert(m_active_dialog != nullptr,
                               "canvas in drag mode but no widgets active");

                m_mouse_mode = MouseMode::Propagate;
                if (drop_widget != m_active_dialog)
                {
                    LocalTransform transform{ m_active_dialog->parent() };
                    m_redraw |= m_active_dialog->on_mouse_button_released(mouse, kb);
                }

                bool drag_btn_released{ mouse.is_button_released(Mouse::Button::Left) };
                if (drag_btn_released)
                    m_active_dialog = nullptr;

                break;
            }
            case MouseMode::Resize:
            {
                runtime_assert(m_active_dialog != nullptr,
                               "canvas in resize mode but no widgets active");

                LocalTransform transform{ m_active_dialog->parent() };
                m_redraw |= m_active_dialog->on_mouse_button_released(mouse, kb);
                m_mouse_mode = MouseMode::Propagate;

                bool resize_btn_released{ mouse.is_button_released(Mouse::Button::Left) };
                if (resize_btn_released)
                    m_active_dialog = nullptr;

                break;
            }

            default:
                assert_msg("Invalid/unhandled UI Canvas mouse mode");
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
        scoped_log("move_pos={}", mouse.wheel());

        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1)
        {
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
        scoped_log();
        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_key_pressed(kb);
        return m_redraw;
    }

    bool Canvas::on_key_released(const Keyboard& kb)
    {
        scoped_log();
        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_key_released(kb);
        return m_redraw;
    }

    bool Canvas::on_character_input(const Keyboard& kb)
    {
        scoped_log();
        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_character_input(kb);
        return m_redraw;
    }
}
