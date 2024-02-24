#include <utility>

#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/popup.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/io.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"

namespace rl::ui {
    Canvas::Canvas(const ds::rect<f32>& rect, const Mouse& mouse, const Keyboard& kb,
                   const std::unique_ptr<NVGRenderer>& nvg_renderer)
        : Widget{ nullptr, nvg_renderer }
        , m_mouse{ mouse }
        , m_keyboard{ kb }
    {
        m_pos = ds::point{ 0.0f, 0.0f };
        m_size = rect.size;

        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            m_cursors[static_cast<std::size_t>(i)] = SDL3::SDL_CreateSystemCursor(
                static_cast<Mouse::Cursor::type>(i));

        Widget::set_theme(new Theme{ nvg_renderer->context() });
        this->set_visible(true);

        m_last_interaction = m_timer.elapsed();
    }

    Canvas::~Canvas()
    {
        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            if (m_cursors[i] != nullptr)
                SDL3::SDL_DestroyCursor(m_cursors[static_cast<std::size_t>(i)]);
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
        nvg::begin_frame(context, m_size.width, m_size.height, PIXEL_RATIO);

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
                        ds::point{ (widget->width() / 2.0f), widget->height() + 10.0f },
                };

                nvg::font_face(context, Font::Name::Sans);
                nvg::font_size(context, 20.0f);
                nvg::text_align(context, nvg::Align::NVGAlignLeft | nvg::Align::NVGAlignTop);
                nvg::text_line_height(context, 1.125f);
                nvg::text_bounds(context, pos.x, pos.y, widget->tooltip().c_str(), nullptr,
                                 bounds.data());

                f32 height{ (bounds[2] - bounds[0]) / 2.0f };
                if (height > (tooltip_width / 2.0f))
                {
                    nvg::text_align(context, nvg::Align::NVGAlignCenter | nvg::Align::NVGAlignTop);
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
            visible ? this->show()   //
                    : this->hide();  //
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

    void Canvas::add_update_callback(const std::function<void()>& refresh_func)
    {
        m_update_callbacks.push_back(refresh_func);
    }

    ComponentFormat Canvas::component_format()
    {
        // Return the component format underlying the screen
        runtime_assert(false, "not implemented");
        return 0;
    }

    PixelFormat Canvas::pixel_format()
    {
        // Return the pixel format underlying the screen
        runtime_assert(false, "not implemented");
        return 0;
    }

    bool Canvas::has_depth_buffer()
    {
        // Does the framebuffer have a depth buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool Canvas::has_stencil_buffer()
    {
        // Does the framebuffer have a stencil buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool Canvas::has_float_buffer()
    {
        // Does the framebuffer use a floating point representation
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    std::string Canvas::title() const
    {
        return m_title;
    }

    bool Canvas::tooltip_fade_in_progress() const
    {
        // Is a tooltip currently fading in?
        const f32 elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed < (m_tooltip_delay / 2.0f) || elapsed > (m_tooltip_delay * 2.0f))
            return false;

        // Temporarily increase the frame rate to fade in the tooltip
        const Widget* widget{ this->find_widget(m_mouse.pos()) };
        return widget != nullptr && !widget->tooltip().empty();
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

        Dialog* dialog{ nullptr };
        while (widget != nullptr)
        {
            m_focus_path.push_back(widget);
            Dialog* dlg{ dynamic_cast<Dialog*>(widget) };
            if (dlg != nullptr)
                dialog = dlg;

            widget = widget->parent();
        }

        for (const auto focus_widget : std::ranges::reverse_view{ m_focus_path })
            focus_widget->on_focus_gained();

        // TODO: test code below, used to crash
        if (dialog != nullptr)
            this->move_dialog_to_front(dialog);
    }

    void Canvas::move_dialog_to_front(Dialog* dialog)
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
                if (popup != nullptr && popup->parent_window() == dialog && idx < base_idx)
                {
                    this->move_dialog_to_front(popup);
                    changed = true;
                    break;
                }
            }
        }
    }

    void Canvas::dispose_dialog(const Dialog* dialog)
    {
        const bool match_found{ std::ranges::find_if(m_focus_path, [&](const Widget* w) {
                                    return w == dialog;
                                }) != m_focus_path.end() };
        if (match_found)
            m_focus_path.clear();

        if (m_drag_widget == dialog)
            m_drag_widget = nullptr;

        this->remove_child(dialog);
    }

    void Canvas::center_dialog(Dialog* dialog) const
    {
        if (dialog->size() == ds::dims<f32>::zero())
        {
            auto&& pref_size{ dialog->preferred_size() };
            dialog->set_size(pref_size);
            dialog->perform_layout();
        }

        const ds::dims offset{ ((m_size - dialog->size()) / 2.0f) - m_pos };
        const ds::point position{ offset.width, offset.height };
        dialog->set_position(position);
    }

    bool Canvas::on_moved(const ds::point<f32>& pt)
    {
        scoped_log("{} => {}", ds::rect{ m_pos, m_size }, ds::rect{ pt, m_size });
        this->set_position(std::forward<decltype(pt)>(pt));
        return true;
    }

    bool Canvas::on_resized(const ds::dims<f32>& size)
    {
        scoped_log("{} => {}", ds::rect{ m_pos, m_size }, ds::rect{ m_pos, size / m_pixel_ratio });

        if (math::is_equal(size.area(), 0.0f))
            return false;

        this->set_size({
            size.width / m_pixel_ratio,
            size.height / m_pixel_ratio,
        });

        this->perform_layout();
        if (m_resize_callback != nullptr)
            m_resize_callback(m_size);

        this->redraw();
        return true;
    }

    bool Canvas::on_mouse_move_event(const Mouse& mouse, const Keyboard& kb)
    {
        const ds::point mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        scoped_logger(log_level::trace, "move_pos={}", mouse.pos());
        const ds::point scaled_pos{ mouse_pos / m_pixel_ratio };
        // pnt -= { 1.0f, 2.0f };

        bool handled{ false };
        if (m_drag_active)
        {
            LocalTransform transform{ m_drag_widget->parent() };
            handled = m_drag_widget->on_mouse_drag(mouse, kb);
        }
        else
        {
            const Widget* widget{ this->find_widget(scaled_pos) };
            if (widget != nullptr && widget->cursor() != m_cursor)
            {
                m_cursor = widget->cursor();
                SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
                runtime_assert(widget_cursor != nullptr, "invalid cursor");
                SDL3::SDL_SetCursor(widget_cursor);
            }
        }

        if (!handled)
            handled = Widget::on_mouse_move(mouse, kb);

        m_redraw |= handled;
        return false;
    }

    bool Canvas::on_mouse_button_pressed_event(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn_pressed={}", mouse.button_pressed());
        const ds::point mouse_pos{ mouse.pos() };

        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1)
        {
            const Dialog* dialog{ dynamic_cast<Dialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->modal())
                if (!dialog->contains(mouse_pos))
                    return false;
        }

        const auto drop_widget{ this->find_widget(mouse_pos) };
        if (drop_widget != nullptr && m_cursor != drop_widget->cursor())
        {
            m_cursor = drop_widget->cursor();
            SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
            runtime_assert(widget_cursor != nullptr, "invalid cursor");
            SDL3::SDL_SetCursor(widget_cursor);
        }

        const bool drag_btn_pressed{ mouse.is_button_pressed(Mouse::Button::Left) };
        if (!m_drag_active && drag_btn_pressed)
        {
            m_drag_widget = this->find_widget(mouse_pos);
            if (m_drag_widget == this)
                m_drag_widget = nullptr;

            m_drag_active = m_drag_widget != nullptr;
            if (!m_drag_active)
                this->update_focus(nullptr);
        }

        m_redraw |= Widget::on_mouse_button_pressed(mouse, kb);
        return false;
    }

    bool Canvas::on_mouse_button_released_event(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_released());

        const ds::point mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        if (m_focus_path.size() > 1)
        {
            const Dialog* dialog{ dynamic_cast<Dialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->modal())
                if (!dialog->contains(mouse_pos))
                    return true;
        }

        const Widget* drop_widget{ this->find_widget(mouse_pos) };
        if (m_drag_active && drop_widget != m_drag_widget)
        {
            LocalTransform transform{ m_drag_widget->parent() };
            m_redraw |= m_drag_widget->on_mouse_button_released(mouse, kb);
        }

        if (drop_widget != nullptr && m_cursor != drop_widget->cursor())
        {
            m_cursor = drop_widget->cursor();
            SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
            runtime_assert(widget_cursor != nullptr, "invalid cursor");
            SDL3::SDL_SetCursor(widget_cursor);
        }

        const bool drag_btn_released{ mouse.is_button_released(Mouse::Button::Left) };
        if (m_drag_active && drag_btn_released)
        {
            m_drag_active = false;
            m_drag_widget = nullptr;
        }

        m_redraw |= Widget::on_mouse_button_released(mouse, kb);
        return false;
    }

    bool Canvas::on_mouse_scroll_event(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("move_pos={}", mouse.wheel());

        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1)
        {
            const Dialog* dialog{ dynamic_cast<Dialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->modal())
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
