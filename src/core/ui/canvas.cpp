#include <utility>

#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/popup.hpp"
#include "utils/io.hpp"

namespace rl::ui {

    UICanvas::UICanvas(ds::dims<i32> size, const Mouse& mouse, const Keyboard& kb,
                       const std::unique_ptr<VectorizedRenderer>& nvg_renderer)
        : ui::Widget{ nullptr, nvg_renderer }
        , m_nvg_context{ nvg_renderer ? nvg_renderer->nvg_context() : nullptr }
        , m_mouse_ref{ mouse }
        , m_kb_ref{ kb }
    {
        m_size = size;

        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            m_cursors[i] = SDL3::SDL_CreateSystemCursor(Mouse::Cursor::type(i));

        this->set_visible(true);
        this->set_theme(new ui::Theme(m_nvg_context));
        this->on_mouse_move({}, {});

        m_last_interaction = m_timer.elapsed();
        m_process_events = true;
        m_drag_active = false;
        m_redraw = true;
    }

    UICanvas::~UICanvas()
    {
        for (i32 i = Mouse::Cursor::Arrow; i < Mouse::Cursor::CursorCount; ++i)
            if (m_cursors[i] != nullptr)
                SDL3::SDL_DestroyCursor(m_cursors[i]);
    }

    bool UICanvas::update()
    {
        for (auto&& update_widget_func : m_update_callbacks)
            update_widget_func();

        return m_update_callbacks.size() > 0;
    }

    void UICanvas::perform_layout()
    {
        this->perform_layout(m_nvg_context);
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
        constexpr static f32 PIXEL_RATIO = 1.0f;
        nvgBeginFrame(m_nvg_context, m_size.width, m_size.height, PIXEL_RATIO);

        this->draw(m_nvg_context);

        // ================================================================
        // ================= DEBUG RENDERING ==============================
        // ================================================================

        this->draw_mouse_intersection(m_nvg_context, m_mouse_ref.pos());

        // ================================================================
        // ================================================================
        // ================================================================

        const f32 elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed > m_tooltip_delay)
        {
            const ui::Widget* widget{ this->find_widget(m_mouse_ref.pos()) };
            if (widget != nullptr && !widget->tooltip().empty())
            {
                constexpr i32 tooltip_width{ 150 };
                std::array<f32, 4> bounds = { 0.0f };
                ds::point<i32> pos{ widget->abs_position() +
                                    ds::point<i32>(widget->width() / 2, widget->height() + 10) };

                nvgFontFace(m_nvg_context, font::name::sans);
                nvgFontSize(m_nvg_context, 20.0f);
                nvgTextAlign(m_nvg_context, Text::Alignment::TopLeft);
                nvgTextLineHeight(m_nvg_context, 1.125f);
                nvgTextBounds(m_nvg_context, pos.x, pos.y, widget->tooltip().c_str(), nullptr,
                              bounds.data());

                i32 height{ static_cast<i32>((bounds[2] - bounds[0]) / 2.0f) };
                if (height > tooltip_width / 2)
                {
                    nvgTextAlign(m_nvg_context, Text::Alignment::TopMiddle);
                    nvgTextBoxBounds(m_nvg_context, pos.x, pos.y, tooltip_width,
                                     widget->tooltip().c_str(), nullptr, bounds.data());

                    height = (bounds[2] - bounds[0]) / 2;
                }

                i32 shift{ 0 };
                if (pos.x - height - 8 < 0)
                {
                    // Keep tooltips on screen
                    shift = pos.x - height - 8;
                    pos.x -= shift;
                    bounds[0] -= shift;
                    bounds[2] -= shift;
                }

                nvgGlobalAlpha(m_nvg_context,
                               std::min(1.0f, 2.0f * (elapsed - m_tooltip_delay)) * 0.8f);

                nvgBeginPath(m_nvg_context);
                nvgFillColor(m_nvg_context, rl::Colors::DarkererGrey);
                nvgRoundedRect(m_nvg_context, bounds[0] - 4 - height, bounds[1] - 4,
                               static_cast<i32>(bounds[2] - bounds[0]) + 8,
                               static_cast<i32>(bounds[3] - bounds[1]) + 8, 3);

                const i32 px{ static_cast<i32>((bounds[2] + bounds[0]) / 2) - height + shift };

                nvgMoveTo(m_nvg_context, px, bounds[1] - 10);
                nvgLineTo(m_nvg_context, px + 7, bounds[1] + 1);
                nvgLineTo(m_nvg_context, px - 7, bounds[1] + 1);
                nvgFill(m_nvg_context);

                nvgFillColor(m_nvg_context, rl::Colors::White);
                nvgFontBlur(m_nvg_context, 0.0f);
                nvgTextBox(m_nvg_context, pos.x - height, pos.y, tooltip_width,
                           widget->tooltip().c_str(), nullptr);
            }
        }

        nvgEndFrame(m_nvg_context);

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

    const std::function<void(ds::dims<i32>)>& UICanvas::resize_callback() const
    {
        return m_resize_callback;
    }

    void UICanvas::set_resize_callback(const std::function<void(ds::dims<i32>)>& callback)
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
        if (elapsed < (m_tooltip_delay / 2) || elapsed > (m_tooltip_delay * 2))
            return false;

        // Temporarily increase the frame rate to fade in the tooltip
        const ui::Widget* widget{ this->find_widget(m_mouse_ref.pos()) };
        return widget != nullptr && !widget->tooltip().empty();
    }

    void UICanvas::update_focus(ui::Widget* widget)
    {
        for (auto focus_widget : m_focus_path)
        {
            if (!focus_widget->focused())
                continue;

            focus_widget->on_focus_lost();
        }

        m_focus_path.clear();

        ui::UICanvas* window{ nullptr };
        while (widget != nullptr)
        {
            m_focus_path.push_back(widget);

            ui::UICanvas* as_canvas{ dynamic_cast<ui::UICanvas*>(widget) };
            if (as_canvas != nullptr)
                window = as_canvas;

            widget = widget->parent();
        }

        for (auto focus_widget : std::ranges::reverse_view{ m_focus_path })
            focus_widget->on_focus_gained();

        // TODO: restructure rl::Window to avoid crash when this ends up
        // invalidating iterator in on_mouse_click()...
        //
        // if (window != nullptr)
        //     this->move_dialog_to_front(static_cast<Window*>(window));
    }

    void UICanvas::move_dialog_to_front(ui::Dialog* dialog)
    {
        auto removal_iterator{ std::remove(m_children.begin(), m_children.end(), dialog) };
        m_children.erase(removal_iterator, m_children.end());

        bool changed{ false };
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
                ui::Popup* popup_wnd{ dynamic_cast<ui::Popup*>(m_children[idx]) };
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

    void UICanvas::dispose_dialog(ui::Dialog* window)
    {
        if (std::find(m_focus_path.begin(), m_focus_path.end(), window) != m_focus_path.end())
            m_focus_path.clear();

        if (m_drag_widget == window)
            m_drag_widget = nullptr;

        this->remove_child(window);
    }

    void UICanvas::center_dialog(ui::Dialog* window) const
    {
        if (window->size() == ds::dims<i32>::zero())
        {
            auto&& pref_size{ window->preferred_size(m_nvg_context) };
            window->set_size(pref_size);
            window->perform_layout(m_nvg_context);
        }

        auto&& offset{ (m_size - window->size()) / 2 };
        window->set_position({ offset.width, offset.height });
    }

    bool UICanvas::drop_event(const std::vector<std::string>& filenames)
    {
        // do nothing,
        // derived objects should define
        return false;
    }

    void UICanvas::drop_callback_event(i32 count, const char** filenames)
    {
        std::vector<std::string> arg(count);
        for (i32 i = 0; i < count; ++i)
            arg[i] = filenames[i];

        m_redraw |= this->drop_event(arg);
    }

    bool UICanvas::on_moved(ds::point<i32> pt)
    {
        if constexpr (io::logging::gui_events)
        {
            ds::rect<i32> prev_rect{ m_pos, m_size };
            ds::rect<i32> new_rect{ pt, prev_rect.size };
            log::info("UICanvas::on_moved: {} => {}", prev_rect, new_rect);
        }

        this->set_position(pt);
        return true;
    }

    bool UICanvas::on_resized(ds::dims<i32> size)
    {
        if constexpr (io::logging::gui_events)
        {
            ds::rect<i32> prev_rect{ m_pos, m_size };
            ds::rect<i32> new_rect{ m_pos, size / m_pixel_ratio };
            log::info("UICanvas::on_resized: {} => {}", prev_rect, new_rect);
        }

        if (size.area() == 0)
            return false;

        auto new_size = ds::dims<i32>{
            static_cast<i32>(size.width / m_pixel_ratio),
            static_cast<i32>(size.height / m_pixel_ratio),
        };

        this->set_size(new_size);
        this->perform_layout();
        return this->redraw();
    }

    bool UICanvas::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        m_last_interaction = m_timer.elapsed();
        ds::point<i32> mouse_pos{ mouse.pos() };

        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_button_pressed => {}", mouse);

        if (m_focus_path.size() > 1)
        {
            const ui::Widget* w{ m_focus_path[m_focus_path.size() - 2] };
            const ui::Dialog* window{ dynamic_cast<const ui::Dialog*>(w) };
            if (window != nullptr && window->modal())
            {
                if (!window->contains(mouse_pos))
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
                update_focus(nullptr);
        }

        m_redraw |= ui::Widget::on_mouse_button_pressed(mouse, kb);

        return m_redraw;
    }

    bool UICanvas::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_button_released => {}", mouse);

        ds::point<i32> mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        if (m_focus_path.size() > 1)
        {
            const ui::Widget* w{ m_focus_path[m_focus_path.size() - 2] };
            const ui::Dialog* window{ dynamic_cast<const ui::Dialog*>(w) };
            if (window != nullptr && window->modal())
            {
                if (!window->contains(mouse_pos))
                    return true;
            }
        }

        const auto drop_widget{ this->find_widget(mouse_pos) };
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

        m_redraw |= ui::Widget::on_mouse_button_released(mouse, kb);
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

            m_pos.x = std::max(m_pos.x, 0);
            m_pos.y = std::max(m_pos.y, 0);

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
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_move => {}", mouse);

        ds::point<i32> mouse_pos{ mouse.pos() };
        m_last_interaction = m_timer.elapsed();

        ds::point<i32> pnt{
            static_cast<i32>(std::round(mouse_pos.x / m_pixel_ratio)),
            static_cast<i32>(std::round(mouse_pos.y / m_pixel_ratio)),
        };

        // TODO: ????????
        pnt -= ds::vector2<i32>{ 1, 2 };

        if (m_drag_active)
            ret = m_drag_widget->on_mouse_drag(mouse, kb);
        else
        {
            ui::Widget* widget{ this->find_widget(pnt) };
            if (widget != nullptr && widget->cursor() != m_cursor)
            {
                m_cursor = widget->cursor();
                SDL3::SDL_Cursor* widget_cursor{ m_cursors[m_cursor] };
                runtime_assert(widget_cursor != nullptr, "invalid cursor");
                SDL3::SDL_SetCursor(widget_cursor);
            }
        }

        m_redraw |= ui::Widget::on_mouse_move(mouse, kb);
        return m_redraw;
    }

    bool UICanvas::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_scroll => {}", mouse);

        m_last_interaction = m_timer.elapsed();
        if (m_focus_path.size() > 1)
        {
            auto window{ dynamic_cast<ui::Dialog*>(m_focus_path[m_focus_path.size() - 2]) };
            if (window != nullptr && window->modal())
            {
                if (!window->contains(mouse.pos()))
                    return true;
            }
        }

        m_redraw |= ui::Widget::on_mouse_scroll(mouse, kb);
        return m_redraw;
    }

    bool UICanvas::on_mouse_entered(const Mouse& mouse)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_entered [pos:{}]", mouse.pos());

        return ui::Widget::on_mouse_entered(mouse);
    }

    bool UICanvas::on_mouse_exited(const Mouse& mouse)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_mouse_exited [pos:{}]", mouse.pos());

        return ui::Widget::on_mouse_exited(mouse);
    }

    bool UICanvas::on_focus_gained()
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_focus_gained");

        return ui::Widget::on_focus_gained();
    }

    bool UICanvas::on_focus_lost()
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_focus_lost");

        return ui::Widget::on_focus_lost();
    }

    bool UICanvas::on_key_pressed(const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_key_pressed => {}", kb);

        m_last_interaction = m_timer.elapsed();
        m_redraw |= ui::Widget::on_key_pressed(kb);
        return m_redraw;
    }

    bool UICanvas::on_key_released(const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_key_released => {}", kb);

        m_last_interaction = m_timer.elapsed();
        m_redraw |= ui::Widget::on_key_released(kb);
        return m_redraw;
    }

    bool UICanvas::on_character_input(const Keyboard& kb)
    {
        if constexpr (io::logging::gui_events)
            log::info("UICanvas::on_character_input => {}", kb);

        m_last_interaction = m_timer.elapsed();
        m_redraw |= ui::Widget::on_character_input(kb);
        return m_redraw;
    }

    // bool UICanvas::on_display_scale_changed(const WindowID id)
    //{
    //     m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
    //     sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);

    //    m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
    //    sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]",
    //    m_window_id);

    //    if constexpr (io::logging::gui_events)
    //        log::info("window::on_display_scale_changed [id:{}, ratio:{}, density:{}]", id,
    //                      m_pixel_ratio, m_pixel_density);

    //    return m_pixel_ratio != 0.0f && m_pixel_density != 0.0f;
    //}

    // bool UICanvas::on_display_content_scale_changed(const DisplayID id)
    //{
    //     m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
    //     sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);

    //    m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
    //    sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]",
    //    m_window_id);

    //    if constexpr (io::logging::gui_events)
    //        log::info("window::on_display_content_scale_changed [id:{}, ratio:{},
    //        density:{}]",
    //                      id, m_pixel_ratio, m_pixel_density);

    //    return m_pixel_ratio != 0.0f && m_pixel_density != 0.0f;
    //}

    // bool UICanvas::on_destroyed(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::gui_events)
    //         log::info("window::on_destroyed [id:{}]", id);
    //     return ret;
    // }
}
