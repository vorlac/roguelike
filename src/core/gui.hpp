#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include <imgui.h>

#include "core/ds/rect.hpp"
#include "core/input.hpp"
#include "core/utils/assert.hpp"
#include "thirdparty/free_solid_font_data.hpp"
#include "thirdparty/icons_font_awesome.hpp"

namespace rl
{
    class Window;
    class Display;

    class gui
    {
    public:
        /**
         * @brief Custom initialization. Not needed if you call Setup. Only needed if you want to
         * add custom setup code. must be followed by EndInitImGui Called by
         * imgui_init, and does the first part of setup, before fonts are rendered
         * */
        void pre_init(const rl::Window& window);

        /**
         * @brief Sets up imgui, loads fonts, and loads themes
         * Calls imgui_init and sets the theme. Will install Font awesome by default
         *
         * @param dark_theme when true(default) the dark theme is used, when false the light theme
         * is used
         * */
        void init(const rl::Window& window, bool dark_theme = true);

        /**
         * @brief End Custom initialization. Not needed if you call Setup. Only needed if you want
         * to add custom setup code. must be proceeded by BeginInitImGui Called by
         * imgui_init and does the second part of setup, and renders fonts.
         * */
        void post_init();

        /**
         * @brief Cleanup imgui and unload font atlas
         * Calls imgui_shutdown
         * */
        void teardown();

    public:
        /**
         * @brief Starts a new imgui Frame
         * Calls imgui_newframe, imgui_process_events, and ImGui::NewFrame
         * together
         * */
        void begin(const rl::Window& window, const rl::Display& display);

        /**
         * @brief Starts a new imgui Frame with a specified delta time
         * */
        void begin_delta(const rl::Window& window, const rl::Display& display);

        /**
         * @brief Processes window/input events and renders the gui elements
         * */
        void update(const Window& window, const Display& display);

        /**
         * @brief Forces the font texture atlas to be recomputed and re-cached
         * */
        void reload_fonts();

        /**
         * @brief Ends an imgui frame and submits all imgui drawing to raylib for processing.
         * Calls ImGui:Render, an d imgui_render_draw_data to draw to the current raylib
         * render target
         * */
        void end();

    public:
        /**
         * @brief Draw a texture as an image in an imgui Context* Uses the current imgui Cursor
         * position and the full texture size.
         *
         * @param image The raylib texture to draw
         * */
        void image(raylib::Texture* image);

        /**
         * @brief Draw a texture as an image in an imgui Context at a specific size
         * Uses the current imgui Cursor position and the specified width and height
         * The image will be scaled up or down to fit as needed
         *
         * @param image The raylib texture to draw
         * @param width The width of the drawn image
         * @param height The height of the drawn image
         * */
        void image_size(raylib::Texture* image, i32 width, i32 height);

        /**
         * @brief Draw a texture as an image in an imgui Context at a specific size
         * Uses the current imgui Cursor position and the specified size
         * The image will be scaled up or down to fit as needed
         *
         * @param image The raylib texture to draw
         * @param size The size of drawn image
         * */
        void image_size_v(raylib::Texture* image, raylib::Vector2 size);

        /**
         * @brief Draw a portion texture as an image in an imgui Context at a defined size
         * Uses the current imgui Cursor position and the specified size
         * The image will be scaled up or down to fit as needed
         *
         * @param image The raylib texture to draw
         * @param destWidth The width of the drawn image
         * @param destHeight The height of the drawn image
         * @param sourceRect The portion of the texture to draw as an image. Negative values
         * for the width and height will flip the image
         * */
        void image_rect(raylib::Texture* image,
                        i32 destWidth,
                        i32 destHeight,
                        raylib::Rectangle sourceRect);

        /**
         * @brief Draws a render texture as an image an imgui Context, automatically flipping the Y
         * axis so it will show correctly on screen
         *
         * @param image The render texture to draw
         * */
        void image_render_texture(raylib::RenderTexture* image);

        /**
         * @brief Draws a render texture as an image an imgui Context, automatically flipping the Y
         * axis so it will show correctly on screen Fits the render texture to the available content
         * area
         *
         * @param image The render texture to draw
         * @param center When true the image will be centered in the content area
         * */
        void image_render_texture_fit(raylib::RenderTexture* image, bool center);

        /**
         * @brief Draws a texture as an image button in an imgui context. Uses the current ImGui
         * cursor position and the full size of the texture
         *
         * @param name The display name and imgui ID for the button
         * @param image The texture to draw
         * @returns True if the button was clicked
         * */
        bool image_button(const char* name, raylib::Texture* image);

        /**
         * @brief Draws a texture as an image button in an imgui context. Uses the current ImGui
         * cursor position and the specified size.
         *
         * @param name The display name and imgui ID for the button
         * @param image The texture to draw
         * @param size The size of the button
         * @returns true if the button was clicked
         * */
        bool image_button_size(const char* name, raylib::Texture* image, ImVec2 size);

    private:
        bool imgui_init(const rl::Window& window);
        void imgui_new_frame(const rl::Window& window, const rl::Display& display);
        void imgui_render(const ImDrawData* draw_data);
        void imgui_process_events(const rl::Window& window);
        void imgui_reload_fonts();
        void imgui_shutdown();

        void imgui_triangle_vert(ImDrawVert& idx_vert);
        void imgui_render_triangles(i32 count,
                                    i32 indexStart,
                                    const ImVector<ImDrawIdx>& indexBuffer,
                                    const ImVector<ImDrawVert>& vertBuffer,
                                    void* texture_data);

    private:
        void enable_scissor(f32 x, f32 y, f32 width, f32 height);
        void setup_font_awesome();
        void setup_imgui_backend();
        void reset_gui_state(const rl::Window& window);

    private:
        rl::Input m_input{};

        bool m_last_frame_focused{ false };
        bool m_last_control_pressed{ false };
        bool m_last_shift_pressed{ false };
        bool m_last_alt_pressed{ false };
        bool m_last_super_pressed{ false };

        ImGuiContext* m_context{ nullptr };
        ImGuiMouseCursor m_curr_mouse_cursor{ ImGuiMouseCursor_COUNT };

        static constexpr inline i32 FONT_AWESOME_ICON_SIZE{ 12 };

        static constexpr inline std::array m_mouse_cursor_map = {
            input::Mouse::Cursor::Arrow,         // ImGuiMouseCursor_Arrow      (0)
            input::Mouse::Cursor::IBeam,         // ImGuiMouseCursor_TextInput  (1)
            input::Mouse::Cursor::OmniResize,    // ImGuiMouseCursor_ResizeAll  (2)
            input::Mouse::Cursor::VertResize,    // ImGuiMouseCursor_ResizeNS   (3)
            input::Mouse::Cursor::HorizResize,   // ImGuiMouseCursor_ResizeEW   (4)
            input::Mouse::Cursor::TRtoBLResize,  // ImGuiMouseCursor_ResizeNESW (5)
            input::Mouse::Cursor::TLtoBRResize,  // ImGuiMouseCursor_ResizeNWSE (6)
            input::Mouse::Cursor::Hand,          // ImGuiMouseCursor_Hand       (7)
            input::Mouse::Cursor::Disabled,      // ImGuiMouseCursor_NotAllowed (8)
        };

        static const inline std::unordered_map<input::Key, ImGuiKey> m_imgui_keymap = {
            { input::Key::Apostrophe, ImGuiKey_Apostrophe },
            { input::Key::Comma, ImGuiKey_Comma },
            { input::Key::Minus, ImGuiKey_Minus },
            { input::Key::Period, ImGuiKey_Period },
            { input::Key::ForwardSlash, ImGuiKey_Slash },
            { input::Key::Zero, ImGuiKey_0 },
            { input::Key::One, ImGuiKey_1 },
            { input::Key::Two, ImGuiKey_2 },
            { input::Key::Three, ImGuiKey_3 },
            { input::Key::Four, ImGuiKey_4 },
            { input::Key::Five, ImGuiKey_5 },
            { input::Key::Six, ImGuiKey_6 },
            { input::Key::Seven, ImGuiKey_7 },
            { input::Key::Eight, ImGuiKey_8 },
            { input::Key::Nine, ImGuiKey_9 },
            { input::Key::Semicolon, ImGuiKey_Semicolon },
            { input::Key::Equal, ImGuiKey_Equal },
            { input::Key::A, ImGuiKey_A },
            { input::Key::B, ImGuiKey_B },
            { input::Key::C, ImGuiKey_C },
            { input::Key::D, ImGuiKey_D },
            { input::Key::E, ImGuiKey_E },
            { input::Key::F, ImGuiKey_F },
            { input::Key::G, ImGuiKey_G },
            { input::Key::H, ImGuiKey_H },
            { input::Key::I, ImGuiKey_I },
            { input::Key::J, ImGuiKey_J },
            { input::Key::K, ImGuiKey_K },
            { input::Key::L, ImGuiKey_L },
            { input::Key::M, ImGuiKey_M },
            { input::Key::N, ImGuiKey_N },
            { input::Key::O, ImGuiKey_O },
            { input::Key::P, ImGuiKey_P },
            { input::Key::Q, ImGuiKey_Q },
            { input::Key::R, ImGuiKey_R },
            { input::Key::S, ImGuiKey_S },
            { input::Key::T, ImGuiKey_T },
            { input::Key::U, ImGuiKey_U },
            { input::Key::V, ImGuiKey_V },
            { input::Key::W, ImGuiKey_W },
            { input::Key::X, ImGuiKey_X },
            { input::Key::Y, ImGuiKey_Y },
            { input::Key::Z, ImGuiKey_Z },
            { input::Key::Space, ImGuiKey_Space },
            { input::Key::Escape, ImGuiKey_Escape },
            { input::Key::Enter, ImGuiKey_Enter },
            { input::Key::Tab, ImGuiKey_Tab },
            { input::Key::Backspace, ImGuiKey_Backspace },
            { input::Key::Insert, ImGuiKey_Insert },
            { input::Key::Delete, ImGuiKey_Delete },
            { input::Key::Right, ImGuiKey_RightArrow },
            { input::Key::Left, ImGuiKey_LeftArrow },
            { input::Key::Down, ImGuiKey_DownArrow },
            { input::Key::Up, ImGuiKey_UpArrow },
            { input::Key::PageUp, ImGuiKey_PageUp },
            { input::Key::PageDown, ImGuiKey_PageDown },
            { input::Key::Home, ImGuiKey_Home },
            { input::Key::End, ImGuiKey_End },
            { input::Key::CapsLock, ImGuiKey_CapsLock },
            { input::Key::ScrollLock, ImGuiKey_ScrollLock },
            { input::Key::NumLock, ImGuiKey_NumLock },
            { input::Key::PrintScreen, ImGuiKey_PrintScreen },
            { input::Key::Pause, ImGuiKey_Pause },
            { input::Key::F1, ImGuiKey_F1 },
            { input::Key::F2, ImGuiKey_F2 },
            { input::Key::F3, ImGuiKey_F3 },
            { input::Key::F4, ImGuiKey_F4 },
            { input::Key::F5, ImGuiKey_F5 },
            { input::Key::F6, ImGuiKey_F6 },
            { input::Key::F7, ImGuiKey_F7 },
            { input::Key::F8, ImGuiKey_F8 },
            { input::Key::F9, ImGuiKey_F9 },
            { input::Key::F10, ImGuiKey_F10 },
            { input::Key::F11, ImGuiKey_F11 },
            { input::Key::F12, ImGuiKey_F12 },
            { input::Key::LeftShift, ImGuiKey_LeftShift },
            { input::Key::LeftCtrl, ImGuiKey_LeftCtrl },
            { input::Key::LeftAlt, ImGuiKey_LeftAlt },
            { input::Key::LeftSuper, ImGuiKey_LeftSuper },
            { input::Key::LeftShift, ImGuiKey_RightShift },
            { input::Key::RightCtrl, ImGuiKey_RightCtrl },
            { input::Key::RightAlt, ImGuiKey_RightAlt },
            { input::Key::RightSuper, ImGuiKey_RightSuper },
            { input::Key::KB_Menu, ImGuiKey_Menu },
            { input::Key::LeftBracket, ImGuiKey_LeftBracket },
            { input::Key::Backslash, ImGuiKey_Backslash },
            { input::Key::RightBracket, ImGuiKey_RightBracket },
            { input::Key::Tilda, ImGuiKey_GraveAccent },
            { input::Key::NP_0, ImGuiKey_Keypad0 },
            { input::Key::NP_1, ImGuiKey_Keypad1 },
            { input::Key::NP_2, ImGuiKey_Keypad2 },
            { input::Key::NP_3, ImGuiKey_Keypad3 },
            { input::Key::NP_4, ImGuiKey_Keypad4 },
            { input::Key::NP_5, ImGuiKey_Keypad5 },
            { input::Key::NP_6, ImGuiKey_Keypad6 },
            { input::Key::NP_7, ImGuiKey_Keypad7 },
            { input::Key::NP_8, ImGuiKey_Keypad8 },
            { input::Key::NP_9, ImGuiKey_Keypad9 },
            { input::Key::NP_Decimal, ImGuiKey_KeypadDecimal },
            { input::Key::NP_Divide, ImGuiKey_KeypadDivide },
            { input::Key::NP_Multiply, ImGuiKey_KeypadMultiply },
            { input::Key::NP_Subtract, ImGuiKey_KeypadSubtract },
            { input::Key::NP_Add, ImGuiKey_KeypadAdd },
            { input::Key::NP_Enter, ImGuiKey_KeypadEnter },
            { input::Key::NP_Equal, ImGuiKey_KeypadEqual },
        };
    };
}
