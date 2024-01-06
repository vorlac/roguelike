#include <glad/gl.h>

#include <vector>

#include <nanovg.h>

#include "core/assert.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/popup.hpp"
#include "core/ui/screen.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "utils/io.hpp"

namespace rl::ui {
    Screen::Screen(NVGcontext* nvg_context, ds::dims<i32> size)
        : ui::widget{ nullptr }
        , m_nvg_context{ nvg_context }
    {
        m_size = size;

        i32 depth_bits{ 0 };
        glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);
        m_depth_buffer = depth_bits > 0;

        i32 stencil_bits{ 0 };
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil_bits);
        m_stencil_buffer = stencil_bits > 0;

        u8 float_mode{ 0 };
        glGetBooleanv(GL_RGBA_FLOAT_MODE_ARB, &float_mode);
        m_float_buffer = float_mode != 0;

        this->init();
    }

    bool Screen::init()
    {
        this->set_visible(true);
        this->set_theme(new ui::theme(m_nvg_context));
        this->on_mouse_move({}, {});

        m_last_interaction = m_timer.elapsed();
        m_process_events = true;
        m_drag_active = false;
        m_redraw = true;
        return true;
    }

    bool Screen::refresh()
    {
        for (auto&& refresh_widget_func : m_refresh_callbacks)
            refresh_widget_func();

        return m_refresh_callbacks.size() > 0;
    }

    bool Screen::draw_setup()
    {
        bool ret{ true };
        // moved into Window::render_start()
        return ret;
    }

    bool Screen::draw_contents()
    {
        this->refresh();
        this->perform_layout();
        return true;
    }

    bool Screen::draw_widgets(const Mouse& mouse, const Keyboard& kb)
    {
        constexpr static f32 PIXEL_RATIO = 1.0f;
        nvgBeginFrame(m_nvg_context, m_size.width, m_size.height, PIXEL_RATIO);

        this->draw(m_nvg_context);

        f32 elapsed{ m_timer.elapsed() - m_last_interaction };
        if (elapsed > m_tooltip_delay)
        {
            const ui::widget* widget{ this->find_widget(mouse.pos()) };
            if (widget && !widget->tooltip().empty())
            {
                i32 tooltip_width{ 150 };
                std::array<f32, 4> bounds{};

                nvgFontFace(m_nvg_context, font::name::sans);
                nvgFontSize(m_nvg_context, 15.0f);
                nvgTextAlign(m_nvg_context, NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_TOP);
                nvgTextLineHeight(m_nvg_context, 1.1f);

                ds::point<i32> pos{
                    widget->abs_position() +
                        ds::point<i32>(widget->width() / 2, widget->height() + 10),
                };

                nvgTextBounds(m_nvg_context, pos.x, pos.y, widget->tooltip().c_str(), nullptr,
                              bounds.data());

                i32 height{ static_cast<i32>((bounds[2] - bounds[0]) / 2.0f) };
                if (height > tooltip_width / 2)
                {
                    nvgTextAlign(m_nvg_context,
                                 NVGalign::NVG_ALIGN_CENTER | NVGalign::NVG_ALIGN_TOP);
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

                nvgGlobalAlpha(m_nvg_context, std::min(1.0f, 2.0f * (elapsed - 0.5f)) * 0.8f);

                nvgBeginPath(m_nvg_context);
                nvgFillColor(m_nvg_context, ds::color<f32>{ 0.0f, 0.0f, 0.0f, 1.0f });
                nvgRoundedRect(m_nvg_context, bounds[0] - 4 - height, bounds[1] - 4,
                               static_cast<i32>(bounds[2] - bounds[0]) + 8,
                               static_cast<i32>(bounds[3] - bounds[1]) + 8, 3);

                i32 px{ static_cast<i32>((bounds[2] + bounds[0]) / 2) - height + shift };

                nvgMoveTo(m_nvg_context, px, bounds[1] - 10);
                nvgLineTo(m_nvg_context, px + 7, bounds[1] + 1);
                nvgLineTo(m_nvg_context, px - 7, bounds[1] + 1);
                nvgFill(m_nvg_context);

                nvgFillColor(m_nvg_context, ds::color<f32>{ 1.0f, 1.0f, 1.0f, 1.0f });
                nvgFontBlur(m_nvg_context, 0.0f);
                nvgTextBox(m_nvg_context, pos.x - height, pos.y, tooltip_width,
                           widget->tooltip().c_str(), nullptr);
            }
        }

        nvgEndFrame(m_nvg_context);

        return true;
    }

    bool Screen::redraw()
    {
        m_redraw |= true;
        return true;
    }

    bool Screen::draw_teardown()
    {
        // moved to Window::render_end()
        return true;
    }

    bool Screen::draw_all(const Mouse& mouse, const Keyboard& kb)
    {
        bool ret = true;
        ret &= this->draw_setup();
        ret &= this->draw_contents();
        ret &= this->draw_widgets(mouse, kb);
        ret &= this->draw_teardown();
        return ret;
    }

    void Screen::draw(NVGcontext* ctx)
    {
        i32 ds = m_theme->m_window_drop_shadow_size, cr = m_theme->m_window_corner_radius;
        i32 hh = m_theme->m_window_header_height;

        // Draw window
        nvgSave(ctx);
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height, cr);
        nvgFillColor(ctx, m_mouse_focus ? m_theme->m_window_fill_focused
                                        : m_theme->m_window_fill_unfocused);
        nvgFill(ctx);

        // Draw a drop shadow
        NVGpaint shadow_paint = nvgBoxGradient(ctx, m_pos.x, m_pos.y, m_size.width, m_size.height,
                                               cr * 2, ds * 2, m_theme->m_drop_shadow,
                                               m_theme->m_transparent);

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
            /* Draw header */
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
            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

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

        ui::widget::draw(ctx);
    }

    void Screen::set_visible(bool visible)
    {
        if (m_visible != visible)
        {
            m_visible = visible;
            if (visible)
                this->show();
            else
                this->hide();
        }
    }

    ds::dims<i32> Screen::frame_buffer_size() const
    {
        // Return the framebuffer size (potentially larger than size() on high-DPI screens)
        // TODO: is ds::dims<i32> get_render_size() good equivalent?
        return m_framebuf_size;
    }

    const std::function<void(ds::dims<i32>)>& Screen::resize_callback() const
    {
        return m_resize_callback;
    }

    void Screen::set_resize_callback(const std::function<void(ds::dims<i32>)>& callback)
    {
        m_resize_callback = callback;
    }

    void Screen::add_refresh_callback(const std::function<void()>& refresh_func)
    {
        m_refresh_callbacks.push_back(refresh_func);
    }

    // Return the component format underlying the screen
    ComponentFormat Screen::component_format() const
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
    PixelFormat Screen::pixel_format() const
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

    bool Screen::has_depth_buffer() const
    {
        // Does the framebuffer have a depth buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool Screen::has_stencil_buffer() const
    {
        // Does the framebuffer have a stencil buffer
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    bool Screen::has_float_buffer() const
    {
        // Does the framebuffer use a floating point representation
        // TODO: call opengl to confirm for debug builds
        return true;
    }

    // Flush all queued up NanoVG rendering commands
    // TODO: move into renderer
    void Screen::nvg_flush()
    {
        runtime_assert(false, "not implemented");
    }

    // Is a tooltip currently fading in?
    bool Screen::tooltip_fade_in_progress() const
    {
        runtime_assert(false, "not implemented");
        return false;
    }

    // Compute the layout of all widgets
    void Screen::perform_layout()
    {
        // using ui::widget::perform_layout(ctx) here...
        ui::widget::perform_layout(m_nvg_context);
    }

    void Screen::update_focus(ui::widget* widget)
    {
        for (auto w : m_focus_path)
        {
            if (!w->focused())
                continue;

            w->on_focus_lost();
        }

        m_focus_path.clear();

        Window* window{ nullptr };
        while (widget != nullptr)
        {
            m_focus_path.push_back(widget);

            Window* as_window{ dynamic_cast<Window*>(widget) };
            if (as_window != nullptr)
                window = as_window;

            widget = widget->parent();
        }

        for (auto it = m_focus_path.rbegin(); it != m_focus_path.rend(); ++it)
        {
            ui::widget* focus_widget{ *it };
            focus_widget->on_focus_gained();
        }

        // TODO: restructure rl::Window to avoid crash when this ends up
        // invalidating iterator in on_mouse_click()...
        //
        // if (window != nullptr)
        //     this->move_window_to_front(static_cast<Window*>(window));
    }

    void Screen::move_window_to_front(Window* window)
    {
        m_children.erase(std::remove(m_children.begin(), m_children.end(), window),
                         m_children.end());

        bool changed{ false };
        m_children.push_back(window);

        do
        {
            // Brute force topological sort (no problem for a few windows..)
            size_t base_idx{ 0 };
            for (size_t idx = 0; idx < m_children.size(); ++idx)
                if (m_children[idx] == window)
                    base_idx = idx;

            changed = false;
            for (size_t idx = 0; idx < m_children.size(); ++idx)
            {
                ui::Popup* popup_wnd{ dynamic_cast<ui::Popup*>(m_children[idx]) };
                if (popup_wnd != nullptr && popup_wnd->parent_window() == window && idx < base_idx)
                {
                    this->move_window_to_front(popup_wnd);
                    changed = true;
                    break;
                }
            }
        }
        while (changed);
    }

    void Screen::dispose_window(Window* window)
    {
        if (std::find(m_focus_path.begin(), m_focus_path.end(), window) != m_focus_path.end())
            m_focus_path.clear();

        if (m_drag_widget == window)
            m_drag_widget = nullptr;

        this->remove_child(window);
    }

    void Screen::center_window(Window* window) const
    {
        if (window->size() == ds::dims<i32>{ 0, 0 })
        {
            auto&& pref_size{ window->preferred_size(m_nvg_context) };
            window->set_size(pref_size);
            window->perform_layout(m_nvg_context);
        }

        auto&& offset{ m_size - window->size() };
        window->set_position({
            offset.width / 2,
            offset.height / 2,
        });
    }

    bool Screen::drop_event(const std::vector<std::string>& filenames)
    {
        // do nothing,
        // derived objects should define
        return false;
    }

    void Screen::drop_callback_event(i32 count, const char** filenames)
    {
        std::vector<std::string> arg(count);
        for (int i = 0; i < count; ++i)
            arg[i] = filenames[i];

        m_redraw |= drop_event(arg);
    }

    // bool Screen::on_moved(WindowID id, ds::point<i32> pt)
    //{
    //     if constexpr (io::logging::window_events)
    //     {
    //         ds::rect<i32> prev_rect{ m_window_rect };
    //         ds::rect<i32> new_rect{ pt, prev_rect.size };
    //         rl::log::info("window::on_moved [id={}] : {} => {}", id, prev_rect, new_rect);
    //     }

    //    bool ret = m_window_rect.pt != pt;
    //    runtime_assert(ret, "window moved, but location unchanged");
    //    m_window_rect.pt = pt;

    //    return ret;
    //}

    // bool Screen::on_resized(ds::dims<i32> size)
    //{
    //     if constexpr (io::logging::window_events)
    //     {
    //         ds::rect<i32> prev_rect{ m_window_rect };
    //         ds::rect<i32> new_rect{ prev_rect.pt, size };
    //         rl::log::info("window::on_resized: {} => {}", prev_rect, new_rect);
    //     }

    //    if (m_resize_callback != nullptr)
    //        m_resize_callback(size);

    //    m_redraw = true;
    //    return this->draw_all();
    //}

    bool Screen::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_mouse_button_pressed [button:{}]", mouse.button_pressed());

        if (ui::widget::on_mouse_button_pressed(mouse, kb))
            return true;

        else if (mouse.is_button_pressed(Mouse::Button::Left))
        {
            i32 offset_height{ mouse.pos().y - m_pos.y };
            m_drag_active = offset_height < m_theme->m_window_header_height;
            return true;
        }

        return false;
    }

    bool Screen::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_mouse_button_released [button:{}]", mouse.button_released());

        if (ui::widget::on_mouse_button_released(mouse, kb))
            return true;

        if (mouse.is_button_released(Mouse::Button::Left))
        {
            m_drag_active = false;
            return true;
        }

        return false;
    }

    bool Screen::on_mouse_drag(ds::point<i32> pnt, ds::vector2<i32> rel, const Mouse& mouse,
                               const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_mouse_drag [pt:{}, rel:{}, btn:{}, mod:{}]", mouse.pos(),
                          mouse.pos_delta(), mouse.button_pressed(),
                          kb.is_button_down(Keyboard::Button::Modifiers));

        if (m_drag_active && mouse.is_button_held(Mouse::Button::Left))
        {
            m_pos += rel;

            m_pos.x = std::max(m_pos.x, 0);
            m_pos.y = std::max(m_pos.y, 0);

            auto relative_size{ this->parent()->size() - m_size };

            m_pos.x = std::min(m_pos.x, relative_size.width);
            m_pos.y = std::min(m_pos.y, relative_size.height);

            return true;
        }

        return false;
    }

    bool Screen::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_mouse_move [pt:{}, rel:{}, btn:{}, mod:{}]", mouse.pos(),
                          mouse.pos_delta(), mouse.button_pressed(),
                          kb.is_button_down(Keyboard::Button::Modifiers));

        return ui::widget::on_mouse_move(mouse, kb);
    }

    bool Screen::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_mouse_scroll [pos:{}, rel:{}]", mouse.pos(), mouse.wheel());

        return ui::widget::on_mouse_scroll(mouse, kb);
    }

    bool Screen::on_mouse_entered(const Mouse& mouse)
    {
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_entered [pos:{}]", mouse.pos());

        return ui::widget::on_mouse_entered(mouse);
    }

    bool Screen::on_mouse_exited(const Mouse& mouse)
    {
        if constexpr (io::logging::window_events)
            log::info("window::on_mouse_exited [pos:{}]", mouse.pos());

        return ui::widget::on_mouse_exited(mouse);
    }

    bool Screen::on_focus_gained()
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_focus_gained");

        return ui::widget::on_focus_gained();
    }

    bool Screen::on_focus_lost()
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_focus_lost");

        return ui::widget::on_focus_lost();
    }

    bool Screen::on_key_pressed(const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_focus_lost");

        return ui::widget::on_key_pressed(kb);
    }

    bool Screen::on_key_released(const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_focus_lost");

        return ui::widget::on_key_released(kb);
    }

    bool Screen::on_character_input(const Keyboard& kb)
    {
        if constexpr (io::logging::window_events)
            rl::log::info("window::on_focus_lost");

        return ui::widget::on_character_input(kb);
    }

    // bool Screen::on_display_scale_changed(const WindowID id)
    //{
    //     m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
    //     sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);

    //    m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
    //    sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]",
    //    m_window_id);

    //    if constexpr (io::logging::window_events)
    //        rl::log::info("window::on_display_scale_changed [id:{}, ratio:{}, density:{}]", id,
    //                      m_pixel_ratio, m_pixel_density);

    //    return m_pixel_ratio != 0.0f && m_pixel_density != 0.0f;
    //}

    // bool Screen::on_display_content_scale_changed(const DisplayID id)
    //{
    //     m_pixel_ratio = SDL3::SDL_GetWindowDisplayScale(m_sdl_window);
    //     sdl_assert(m_pixel_ratio != 0.0f, "failed to get pixel ratio [window:{}]", m_window_id);

    //    m_pixel_density = SDL3::SDL_GetWindowPixelDensity(m_sdl_window);
    //    sdl_assert(m_pixel_density != 0.0f, "failed to get pixel density [window:{}]",
    //    m_window_id);

    //    if constexpr (io::logging::window_events)
    //        rl::log::info("window::on_display_content_scale_changed [id:{}, ratio:{},
    //        density:{}]",
    //                      id, m_pixel_ratio, m_pixel_density);

    //    return m_pixel_ratio != 0.0f && m_pixel_density != 0.0f;
    //}

    // bool Screen::on_destroyed(const WindowID id)
    //{
    //     bool ret = true;
    //     if constexpr (io::logging::window_events)
    //         rl::log::info("window::on_destroyed [id:{}]", id);
    //     return ret;
    // }
}
