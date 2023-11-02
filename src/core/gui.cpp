#include <atomic>
#include <vector>

#include <GLFW/glfw3.h>

#include "core/display.hpp"
#include "core/gui.hpp"
#include "core/window.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    void gui::begin(const rl::Window& window, const rl::Display& display)
    {
        ImGui::SetCurrentContext(m_context);
        this->begin_delta(window, display);
    }

    void gui::begin_delta(const rl::Window& window, const rl::Display& display)
    {
        ImGui::SetCurrentContext(m_context);
        this->imgui_new_frame(window, display);
        this->imgui_impl_process_events(window);
        ImGui::NewFrame();
    }

    bool gui::imgui_impl_process_events(const rl::Window& window)
    {
        ImGuiIO& io{ ImGui::GetIO() };
        const bool focused = window.is_focused();

        const bool ctrl_down = m_input.keyboard.is_key_down(input::Key::LeftCtrl) ||
                               m_input.keyboard.is_key_down(input::Key::RightCtrl);
        const bool shift_down = m_input.keyboard.is_key_down(input::Key::LeftShift) ||
                                m_input.keyboard.is_key_down(input::Key::RightShift);
        const bool alt_down = m_input.keyboard.is_key_down(input::Key::LeftAlt) ||
                              m_input.keyboard.is_key_down(input::Key::RightAlt);
        const bool super_down = m_input.keyboard.is_key_down(input::Key::LeftSuper) ||
                                m_input.keyboard.is_key_down(input::Key::RightSuper);

        if (focused != m_last_frame_focused)
            io.AddFocusEvent(focused);
        if (ctrl_down != m_last_control_pressed)
            io.AddKeyEvent(ImGuiMod_Ctrl, ctrl_down);
        if (shift_down != m_last_shift_pressed)
            io.AddKeyEvent(ImGuiMod_Shift, shift_down);
        if (alt_down != m_last_alt_pressed)
            io.AddKeyEvent(ImGuiMod_Alt, alt_down);
        if (super_down != m_last_super_pressed)
            io.AddKeyEvent(ImGuiMod_Super, super_down);

        m_last_frame_focused   = focused;
        m_last_control_pressed = ctrl_down;
        m_last_shift_pressed   = shift_down;
        m_last_alt_pressed     = alt_down;
        m_last_super_pressed   = super_down;

        // get the pressed keys, they are in event order
        input::Key key_id{ m_input.keyboard.get_key_pressed() };
        while (key_id != input::Key::Null)
        {
            const auto keymap{ m_imgui_keymap.find(key_id) };
            if (keymap != m_imgui_keymap.end())
                io.AddKeyEvent(keymap->second, true);
            key_id = m_input.keyboard.get_key_pressed();
        }

        // look for any keys that were down last frame and see if they were down and are released
        for (const auto& keymap : m_imgui_keymap)
            if (m_input.keyboard.is_key_released(keymap.first))
                io.AddKeyEvent(keymap.second, false);

        // add the text input in order
        u32 pressed = m_input.keyboard.get_char_pressed();
        while (pressed != 0)
        {
            io.AddInputCharacter(pressed);
            pressed = m_input.keyboard.get_char_pressed();
        }

        return true;
    }

    void gui::imgui_new_frame(const rl::Window& window, const rl::Display& display)
    {
        ImGuiIO& io{ ImGui::GetIO() };
        if (!window.is_fullscreen())
            io.DisplaySize = window.screen_size();
        else
        {
            const i32 monitor{ display.current_monitor() };
            io.DisplaySize = display.monitor_dims(monitor);
        }

        i32 width{ cast::to<i32>(io.DisplaySize.x) };
        i32 height{ cast::to<i32>(io.DisplaySize.y) };
        GLFWwindow* gflw_window{ glfwGetCurrentContext() };
        glfwGetFramebufferSize(gflw_window, &width, &height);

        if (width <= 0 || height <= 0)
            io.DisplayFramebufferScale = { 1.0f, 1.0f };
        else
        {
            io.DisplayFramebufferScale = {
                cast::to<f32>(width) / io.DisplaySize.x,
                cast::to<f32>(height) / io.DisplaySize.y,
            };
        }

        io.DeltaTime = window.frame_time();
        if (!io.WantSetMousePos) [[likely]]
            io.MousePos = m_input.mouse.get_position();
        else [[unlikely]]
        {
            // rarely happens
            m_input.mouse.set_position(io.MousePos.x, io.MousePos.y);
        }

        io.MouseDown[0] = m_input.mouse.is_button_down(input::Mouse::Button::Left);
        io.MouseDown[1] = m_input.mouse.is_button_down(input::Mouse::Button::Right);
        io.MouseDown[2] = m_input.mouse.is_button_down(input::Mouse::Button::Middle);

        {
            auto&& mw_delta{ m_input.mouse.get_wheel_move_v() };
            io.MouseWheel += mw_delta.y;
            io.MouseWheelH += mw_delta.x;
        }

        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
        {
            ImGuiMouseCursor imgui_cursor{ ImGui::GetMouseCursor() };
            if (imgui_cursor != m_curr_mouse_cursor || io.MouseDrawCursor)
            {
                m_curr_mouse_cursor = imgui_cursor;
                if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
                    m_input.mouse.hide_cursor();
                else
                {
                    m_input.mouse.show_cursor();
                    if (!(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
                    {
                        input::Mouse::Cursor cursor_type =
                            (imgui_cursor > -1 &&
                             imgui_cursor < input::Mouse::Cursor::MouseCursorCount)
                                ? m_mouse_cursor_map[cast::to<u64>(imgui_cursor)]
                                : input::Mouse::Cursor::Default;

                        m_input.mouse.set_cursor(cursor_type);
                    }
                }
            }
        }
    }

    bool gui::imgui_impl_init(const rl::Window& window)
    {
        this->reset_gui_state(window);

        ImGuiIO& io{ ImGui::GetIO() };
        io.Fonts->AddFontDefault();

        this->setup_font_awesome();
        this->setup_imgui_backend();
        this->reload_fonts_internal();

        return true;
    }

    void gui::setup(const rl::Window& window, bool dark_theme /* = true*/)
    {
        this->begin_init_imgui(window);
        dark_theme ? ImGui::StyleColorsDark()  //
                   : ImGui::StyleColorsLight();
        this->end_init_imgui();
    }

    void gui::begin_init_imgui(const rl::Window& window)
    {
        this->reset_gui_state(window);
        m_context = ImGui::CreateContext();

        ImGuiIO& io{ ImGui::GetIO() };
        io.Fonts->AddFontDefault();
    }

    void gui::reset_gui_state(const rl::Window& window)
    {
        m_last_frame_focused   = window.is_focused();
        m_last_control_pressed = false;
        m_last_shift_pressed   = false;
        m_last_alt_pressed     = false;
        m_last_super_pressed   = false;
    }
}
