#include <utility>

#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/popup.hpp"
#include "utils/io.hpp"

namespace rl::ui {

    UICanvas::UICanvas(ds::rect<f32> rect, const Mouse& mouse, const Keyboard& kb,
                       const std::unique_ptr<NVGRenderer>& vg_renderer)
        : Widget{ nullptr, vg_renderer }
        , m_mouse{ mouse }
        , m_keyboard{ kb }
    {
        m_pos = ds::point{ 0.0f, 0.0f };
        m_size = rect.size;

        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            m_cursors[i] = SDL3::SDL_CreateSystemCursor(Mouse::Cursor::type(i));

        this->set_visible(true);
        this->set_theme(new Theme{ vg_renderer->context() });
        this->on_mouse_move({}, {});

        m_last_interaction = m_timer.elapsed();
    }

    UICanvas::~UICanvas()
    {
        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            if (m_cursors[i] != nullptr)
                SDL3::SDL_DestroyCursor(m_cursors[i]);
    }

    bool UICanvas::update()
    {
        for (auto update_widget_func : m_update_callbacks)
            update_widget_func();

        return m_update_callbacks.size() > 0;
    }

    bool UICanvas::draw_setup()
    {
        return true;
    }

    bool UICanvas::draw_contents()
    {
        this->update();
        return true;
    }

    bool UICanvas::draw_widgets()
    {
        auto&& context{ m_renderer->context() };
        constexpr static f32 PIXEL_RATIO{ 1.0f };
        nvg::BeginFrame(context, m_size.width, m_size.height, PIXEL_RATIO);

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
                ds::point<f32> pos{
                    widget->position() + (widget->width() / 2.0f, widget->height() + 10.0f),
                };

                nvg::FontFace(context, font::name::sans);
                nvg::FontSize(context, 20.0f);
                nvg::TextAlign(context, Text::Alignment::HLeftVTop);
                nvg::TextLineHeight(context, 1.125f);
                nvg::TextBounds(context, pos.x, pos.y, widget->tooltip().c_str(), nullptr,
                                bounds.data());

                f32 height{ (bounds[2] - bounds[0]) / 2.0f };
                if (height > (tooltip_width / 2.0f))
                {
                    nvg::TextAlign(context, Text::Alignment::HMiddleVTop);
                    nvg::TextBoxBounds(context, pos.x, pos.y, tooltip_width,
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

                nvg::GlobalAlpha(context, std::min(1.0f, 2.0f * (elapsed - m_tooltip_delay)) * 0.8f);

                nvg::BeginPath(context);
                nvg::FillColor(context, rl::Colors::DarkererGrey);
                nvg::RoundedRect(context, bounds[0] - 4.0f - height, bounds[1] - 4.0f,
                                 (bounds[2] - bounds[0]) + 8.0f, (bounds[3] - bounds[1]) + 8.0f,
                                 3.0f);

                const f32 px{ ((bounds[2] + bounds[0]) / 2.0f) - height + shift };

                nvg::MoveTo(context, px, bounds[1] - 10);
                nvg::LineTo(context, px + 7, bounds[1] + 1);
                nvg::LineTo(context, px - 7, bounds[1] + 1);
                nvg::Fill(context);

                nvg::FillColor(context, rl::Colors::White);
                nvg::FontBlur(context, 0.0f);
                nvg::TextBox(context, pos.x - height, pos.y, tooltip_width,
                             widget->tooltip().c_str(), nullptr);
            }
        }

        nvg::EndFrame(context);

        return true;
    }

    bool UICanvas::redraw()
    {
        m_redraw |= true;
        return true;
    }

    bool UICanvas::draw_teardown()
    {
        // moved to Window::render_end()
        return true;
    }

    bool UICanvas::draw_all()
    {
        bool ret = true;
        ret &= this->draw_setup();
        ret &= this->draw_contents();
        ret &= this->draw_widgets();
        ret &= this->draw_teardown();
        return ret;
    }

    void UICanvas::set_visible(bool visible)
    {
        if (visible != m_visible)
        {
            m_visible = visible;
            visible ? this->show()   //
                    : this->hide();  //
        }
    }

    ds::dims<i32> UICanvas::frame_buffer_size() const
    {
        // Return the framebuffer size (potentially larger than size() on high-DPI screens)
        // TODO: is ds::dims<i32> get_render_size() good equivalent?
        return m_framebuf_size;
    }

    const std::function<void(ds::dims<f32>)>& UICanvas::resize_callback() const
    {
        return m_resize_callback;
    }

    void UICanvas::set_resize_callback(const std::function<void(ds::dims<f32>)>& callback)
    {
        m_resize_callback = callback;
    }

    void UICanvas::add_update_callback(const std::function<void()>& refresh_func)
    {
        m_update_callbacks.push_back(refresh_func);
    }

    // Return the component format underlying the screen
    ComponentFormat UICanvas::component_format() const
    {
        // Signed and unsigned integer formats
        // ====================================
        // UInt8  = (uint8_t) VariableType::UInt8,
        // Int8   = (uint8_t) VariableType::Int8,
        // UInt16 = (uint16_t) VariableType::UInt16,
        // Int16  = (uint16_t) VariableType::Int16,
        // UInt32 = (uint32_t) VariableType::UInt32,
        // Int32  = (uint32_t) VariableType::Int32,

        // Floating point formats
        // ====================================
        // Float16  = (uint16_t) VariableType::Float16,
        // Float32  = (uint32_t) VariableType::Float32
        runtime_assert(false, "not implemented");
        return 0;
    }

    // Return the pixel format underlying the screen
    PixelFormat UICanvas::pixel_format() const
    {
        // Single-channel bitmap
        //   R,
        // Two-channel bitmap
        //   RA,
        // RGB bitmap
        //   RGB,
        // RGB bitmap + alpha channel
        //   RGBA,
        // BGR bitmap
        //   BGR,
        // BGR bitmap + alpha channel
        //   BGRA,
        // Depth map
        //   Depth,
        // Combined depth + stencil map
        //   DepthStencil
        runtime_assert(false, "not implemented");
        return 0;
    }

    bool UICanvas::has_depth_buffer() const
    {
        // Does the framebuffer have a depth buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool UICanvas::has_stencil_buffer() const
    {
        // Does the framebuffer have a stencil buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool UICanvas::has_float_buffer() const
    {
        // Does the framebuffer use a floating point representation
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    std::string UICanvas::title() const
    {
        return m_title;
    }

    bool UICanvas::tooltip_fade_in_progress() const
    {
        // Is a tooltip currently fading in?
        const f32 elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed < (m_tooltip_delay / 2.0f) || elapsed > (m_tooltip_delay * 2.0f))
            return false;

        // Temporarily increase the frame rate to fade in the tooltip
        const Widget* widget{ this->find_widget(m_mouse.pos()) };
        return widget != nullptr && !widget->tooltip().empty();
    }

    void UICanvas::update_focus(Widget* widget)
    {
        for (auto focus_widget : m_focus_path)
        {
            if (!focus_widget->focused())
                continue;

            focus_widget->on_focus_lost();
        }

        m_focus_path.clear();

        Widget* dialog{ nullptr };
        while (widget != nullptr)
        {
            m_focus_path.push_back(widget);

            UICanvas* as_canvas{ dynamic_cast<UICanvas*>(widget) };
            if (as_canvas != nullptr)
                dialog = as_canvas;

            widget = widget->parent();
        }

        for (auto focus_widget : std::ranges::reverse_view{ m_focus_path })
            focus_widget->on_focus_gained();

        // if (window != nullptr)
        //     this->move_dialog_to_front(static_cast<Dialog*>(dialog));
    }

    void UICanvas::move_dialog_to_front(Dialog* dialog)
    {
        bool changed{ false };
        auto removal_iterator{ std::remove(m_children.begin(), m_children.end(), dialog) };
        m_children.erase(removal_iterator, m_children.end());
        m_children.push_back(dialog);

        do
        {
            // Brute force topological sort (no problem for a few windows..)
            size_t base_idx{ 0 };
            for (size_t idx = 0; idx < m_children.size(); ++idx)
                if (m_children[idx] == dialog)
                    base_idx = idx;

            changed = false;
            for (size_t idx = 0; idx < m_children.size(); ++idx)
            {
                Popup* popup_wnd{ dynamic_cast<Popup*>(m_children[idx]) };
                if (popup_wnd != nullptr && popup_wnd->parent_window() == dialog && idx < base_idx)
                {
                    this->move_dialog_to_front(dialog);
                    changed = true;
                    break;
                }
            }
        }
        while (changed);
    }

    void UICanvas::dispose_dialog(Dialog* dialog)
    {
        bool match_found = std::ranges::find_if(m_focus_path, [&](Widget* w) {
                               return w == dialog;
                           }) != m_focus_path.end();

        if (match_found)
            m_focus_path.clear();

        if (m_drag_widget == dialog)
            m_drag_widget = nullptr;

        this->remove_child(dialog);
    }

    void UICanvas::center_dialog(Dialog* dialog) const
    {
        if (dialog->size() == ds::dims<f32>::zero())
        {
            auto&& pref_size{ dialog->preferred_size() };
            dialog->set_size(pref_size);
            dialog->perform_layout();
        }

        ds::dims<f32> offset{ ((m_size - dialog->size()) / 2.0f) - m_pos };
        ds::point<f32> position{ offset.width, offset.height };
        dialog->set_position(position);
    }

    bool UICanvas::on_moved(ds::point<f32> pt)
    {
        if constexpr (io::logging::gui_events)
        {
            ds::rect<f32> prev_rect{ m_pos, m_size };
            ds::rect<f32> new_rect{ pt, prev_rect.size };
            log::info("UICanvas::on_moved: {} => {}", prev_rect, new_rect);
        }

        this->set_position(pt);
        return true;
    }

    bool UICanvas::on_resized(ds::dims<f32> size)
    {
        if constexpr (io::logging::gui_events)
        {
            ds::rect<f32> prev_rect{ m_pos, m_size };
            ds::rect<f32> new_rect{ m_pos, size / m_pixel_ratio };
            log::info("UICanvas::on_resized: {} => {}", prev_rect, new_rect);
        }

        if (size.area() == 0)
            return false;

        this->set_size({
            size.width / m_pixel_ratio,
            size.height / m_pixel_ratio,
        });

        this->perform_layout();
        return this->redraw();
    }

    bool UICanvas::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        const ds::point<f32> mouse_pos{ mouse.pos() };
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_button_pressed => {}", mouse);

        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1)
        {
            const Widget* w{ m_focus_path[m_focus_path.size() - 2] };
            const Dialog* dialog{ dynamic_cast<const Dialog*>(w) };
            if (dialog != nullptr && dialog->modal())
            {
                if (!dialog->contains(mouse_pos))
                    return true;
            }
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

        return m_redraw;
    }

    bool UICanvas::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_button_released => {}", mouse);

        const ds::point<f32> mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        if (m_focus_path.size() > 1)
        {
            const Widget* w{ m_focus_path[m_focus_path.size() - 2] };
            const Dialog* dialog{ dynamic_cast<const Dialog*>(w) };
            if (dialog != nullptr && dialog->modal())
            {
                if (!dialog->contains(mouse_pos))
                    return true;
            }
        }

        Widget* drop_widget{ this->find_widget(mouse_pos) };
        if (m_drag_active && drop_widget != m_drag_widget)
            m_redraw |= m_drag_widget->on_mouse_button_released(mouse, kb);

        if (m_drag_active && drop_widget != nullptr && m_cursor != drop_widget->cursor())
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
        return m_redraw;
    }

    bool UICanvas::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_drag [pt:{}, rel:{}, btn:{}, mod:{}]", mouse.pos(),
                      mouse.pos_delta(), std::to_underlying(mouse.button_pressed()),
                      kb.is_button_down(Keyboard::Scancode::Modifiers));

        if (m_drag_active && mouse.is_button_held(Mouse::Button::Left))
        {
            m_pos += mouse.pos_delta();

            m_pos.x = std::max(m_pos.x, 0.0f);
            m_pos.y = std::max(m_pos.y, 0.0f);

            auto relative_size{ this->parent()->size() - m_size };

            m_pos.x = std::min(m_pos.x, relative_size.width);
            m_pos.y = std::min(m_pos.y, relative_size.height);

            return true;
        }

        return false;
    }

    bool UICanvas::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        bool ret{ false };
        if constexpr (io::logging::mouse_move_events)
            log::info("UICanvas::on_mouse_move => {}", mouse);

        const ds::point<f32> mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        ds::point<f32> pnt{
            (mouse_pos.x / m_pixel_ratio) - 1.0f,
            (mouse_pos.y / m_pixel_ratio) - 2.0f,
        };

        if (m_drag_active)
            ret = m_drag_widget->on_mouse_drag(mouse, kb);
        else
        {
            Widget* widget{ this->find_widget(pnt) };
            if (widget != nullptr && widget->cursor() != m_cursor)
            {
                m_cursor = widget->cursor();
                SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
                runtime_assert(widget_cursor != nullptr, "invalid cursor");
                SDL3::SDL_SetCursor(widget_cursor);
            }
        }

        m_redraw |= Widget::on_mouse_move(mouse, kb);
        return m_redraw;
    }

    bool UICanvas::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_scroll => {}", mouse);

        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1)
        {
            auto dialog{ dynamic_cast<Dialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (dialog != nullptr && dialog->modal())
            {
                if (!dialog->contains(mouse.pos()))
                    return true;
            }
        }

        m_redraw |= Widget::on_mouse_scroll(mouse, kb);
        return m_redraw;
    }

    bool UICanvas::on_mouse_entered(const Mouse& mouse)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_entered [pos:{}]", mouse.pos());

        return Widget::on_mouse_entered(mouse);
    }

    bool UICanvas::on_mouse_exited(const Mouse& mouse)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_exited [pos:{}]", mouse.pos());

        return Widget::on_mouse_exited(mouse);
    }

    bool UICanvas::on_focus_gained()
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_focus_gained");

        return Widget::on_focus_gained();
    }

    bool UICanvas::on_focus_lost()
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_focus_lost");

        return Widget::on_focus_lost();
    }

    bool UICanvas::on_key_pressed(const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_key_pressed => {}", kb);

        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_key_pressed(kb);
        return m_redraw;
    }

    bool UICanvas::on_key_released(const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_key_released => {}", kb);

        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_key_released(kb);
        return m_redraw;
    }

    bool UICanvas::on_character_input(const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_character_input => {}", kb);

        m_last_interaction = m_timer.elapsed();
        m_redraw |= Widget::on_character_input(kb);
        return m_redraw;
    }
}
