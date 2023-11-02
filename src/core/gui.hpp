#pragma once

#include <utility>
#include <vector>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <unordered_map>

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
         * @brief Sets up ImGui, loads fonts and themes
         * Calls imgui_impl_init and sets the theme. Will install Font awesome by default
         *
         * @param dark_theme when true(default) the dark theme is used, when false the light theme
         * is used
         * */
        void setup(const rl::Window& window, bool dark_theme = true);

        /**
         * @brief Starts a new ImGui Frame
         * Calls imgui_impl_newframe, imgui_impl_process_events, and ImGui::NewFrame
         * together
         * */
        void begin(const rl::Window& window, const rl::Display& display);

        /**
         * @brief Ends an ImGui frame and submits all ImGui drawing to raylib for processing.
         * Calls ImGui:Render, an d imgui_impl_render_draw_data to draw to the current raylib
         * render target
         * */
        void end()
        {
            ImGui::SetCurrentContext(m_context);
            ImGui::Render();

            const ImDrawData* draw_data{ ImGui::GetDrawData() };
            this->imgui_impl_render_draw_data(draw_data);
        }

        /**
         * @brief Cleanup ImGui and unload font atlas
         * Calls imgui_impl_shutdown
         * */
        void teardown()
        {
            ImGui::SetCurrentContext(m_context);
            this->imgui_impl_shutdown();
            ImGui::DestroyContext();
        }

        bool update(const Window& window, const Display& display)
        {
            this->begin(window, display);
            bool open = true;
            ImGui::ShowDemoWindow(&open);
            this->end();
            return true;
        }

        // Advanced StartupAPI

        /**
         * @brief Custom initialization. Not needed if you call Setup. Only needed if you want to
         * add custom setup code. must be followed by EndInitImGui Called by
         * imgui_impl_init, and does the first part of setup, before fonts are rendered
         * */
        void begin_init_imgui(const rl::Window& window);

        /**
         * @brief End Custom initialization. Not needed if you call Setup. Only needed if you want
         * to add custom setup code. must be proceeded by BeginInitImGui Called by
         * imgui_impl_init and does the second part of setup, and renders fonts.
         * */
        void end_init_imgui()
        {
            ImGui::SetCurrentContext(m_context);
            this->setup_font_awesome();
            this->setup_imgui_backend();
            this->reload_fonts_internal();
        }

        /**
         * @brief Forces the font texture atlas to be recomputed and re-cached
         * */
        void reload_fonts()
        {
            ImGui::SetCurrentContext(m_context);
            this->reload_fonts_internal();
        }

        /**
         * @brief Starts a new ImGui Frame with a specified delta time
         * */
        void begin_delta(const rl::Window& window, const rl::Display& display);

        /**
         * ImGui Image API extensions
         * Purely for convenience in working with raylib textures as images.
         * If you want to call ImGui image functions directly, simply pass them the pointer to the
         * texture.
         * */

        /**
         * @brief Draw a texture as an image in an ImGui Context* Uses the current ImGui Cursor
         * position and the full texture size.
         *
         * @param image The raylib texture to draw
         * */
        void image(raylib::Texture* image)
        {
            if (image == nullptr)
                return;
            if (m_context != nullptr)
                ImGui::SetCurrentContext(m_context);

            ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                         ImVec2(cast::to<f32>(image->width), cast::to<f32>(image->height)));
        }

        /**
         * @brief Draw a texture as an image in an ImGui Context at a specific size
         * Uses the current ImGui Cursor position and the specified width and height
         * The image will be scaled up or down to fit as needed
         *
         * @param image The raylib texture to draw
         * @param width The width of the drawn image
         * @param height The height of the drawn image
         * */
        void image_size(raylib::Texture* image, i32 width, i32 height)
        {
            if (image == nullptr)
                return;
            if (m_context != nullptr)
                ImGui::SetCurrentContext(m_context);

            ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                         ImVec2(cast::to<f32>(width), cast::to<f32>(height)));
        }

        /**
         * @brief Draw a texture as an image in an ImGui Context at a specific size
         * Uses the current ImGui Cursor position and the specified size
         * The image will be scaled up or down to fit as needed
         *
         * @param image The raylib texture to draw
         * @param size The size of drawn image
         * */
        void image_size_v(raylib::Texture* image, raylib::Vector2 size)
        {
            if (image == nullptr)
                return;
            if (m_context != nullptr)
                ImGui::SetCurrentContext(m_context);

            ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                         ImVec2(cast::to<f32>(size.x), cast::to<f32>(size.y)));
        }

        /**
         * @brief Draw a portion texture as an image in an ImGui Context at a defined size
         * Uses the current ImGui Cursor position and the specified size
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
                        raylib::Rectangle sourceRect)
        {
            if (image == nullptr)
                return;
            if (m_context != nullptr)
                ImGui::SetCurrentContext(m_context);

            ImVec2 uv0{};
            ImVec2 uv1{};

            if (sourceRect.width < 0)
            {
                uv0.x = -(sourceRect.x / cast::to<f32>(image->width));
                uv1.x = uv0.x - (std::fabs(sourceRect.width) / cast::to<f32>(image->width));
            }
            else
            {
                uv0.x = sourceRect.x / cast::to<f32>(image->width);
                uv1.x = uv0.x + (sourceRect.width / cast::to<f32>(image->width));
            }

            if (sourceRect.height < 0)
            {
                uv0.y = -(sourceRect.y / cast::to<f32>(image->height));
                uv1.y = uv0.y - (std::fabs(sourceRect.height) / cast::to<f32>(image->height));
            }
            else
            {
                uv0.y = sourceRect.y / cast::to<f32>(image->height);
                uv1.y = uv0.y + (sourceRect.height / cast::to<f32>(image->height));
            }

            ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                         ImVec2(cast::to<f32>(destWidth), cast::to<f32>(destHeight)), uv0, uv1);
            // ImGui::Image((ImTextureID)image, ImVec2(float(destWidth), float(destHeight)), uv0, uv1);
        }

        /**
         * @brief Draws a render texture as an image an ImGui Context, automatically flipping the Y
         * axis so it will show correctly on screen
         *
         * @param image The render texture to draw
         * */
        void image_render_texture(raylib::RenderTexture* image)
        {
            if (image == nullptr)
                return;
            if (m_context != nullptr)
                ImGui::SetCurrentContext(m_context);

            this->image_rect(&image->texture,
                             image->texture.width,
                             image->texture.height,
                             ds::rect<f32>{
                                 0,
                                 0,
                                 float(image->texture.width),
                                 -float(image->texture.height),
                             });
        }

        /**
         * @brief Draws a render texture as an image an ImGui Context, automatically flipping the Y
         * axis so it will show correctly on screen Fits the render texture to the available content
         * area
         *
         * @param image The render texture to draw
         * @param center When true the image will be centered in the content area
         * */
        void image_render_texture_fit(raylib::RenderTexture* image, bool center)
        {
            if (image == nullptr)
                return;

            if (this->m_context != nullptr)
                ImGui::SetCurrentContext(this->m_context);

            ImVec2 area{ ImGui::GetContentRegionAvail() };

            f32 scale{ area.x / image->texture.width };
            f32 y{ image->texture.height * scale };
            if (y > area.y)
                scale = area.y / cast::to<f32>(image->texture.height);

            i32 sizeX{ cast::to<i32>(image->texture.width * scale) };
            i32 sizeY{ cast::to<i32>(image->texture.height * scale) };

            if (center)
            {
                ImGui::SetCursorPosX(0.0f);
                ImGui::SetCursorPosX(cast::to<f32>(area.x) / 2.0f - cast::to<f32>(sizeX) / 2.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                                     (cast::to<f32>(area.y) / 2.0f - cast::to<f32>(sizeY) / 2.0f));
            }

            this->image_rect(&image->texture,
                             sizeX,
                             sizeY,
                             ds::rect<f32>{
                                 0.0f,
                                 0.0f,
                                 cast::to<f32>(image->texture.width),
                                 -cast::to<f32>(image->texture.height),
                             });
        }

        /**
         * @brief Draws a texture as an image button in an ImGui context. Uses the current ImGui
         * cursor position and the full size of the texture
         *
         * @param name The display name and ImGui ID for the button
         * @param image The texture to draw
         * @returns True if the button was clicked
         * */
        bool image_button(const char* name, raylib::Texture* image)
        {
            if (image == nullptr)
                return false;
            if (m_context)
                ImGui::SetCurrentContext(m_context);

            return ImGui::ImageButton(
                name,
                static_cast<const ImTextureID>(image),
                ImVec2(cast::to<f32>(image->width), cast::to<f32>(image->height)));
        }

        /**
         * @brief Draws a texture as an image button in an ImGui context. Uses the current ImGui
         * cursor position and the specified size.
         *
         * @param name The display name and ImGui ID for the button
         * @param image The texture to draw
         * @param size The size of the button
         * @returns true if the button was clicked
         * */
        bool image_button_size(const char* name, raylib::Texture* image, ImVec2 size)
        {
            if (image == nullptr)
                return false;
            if (m_context != nullptr)
                ImGui::SetCurrentContext(m_context);

            return ImGui::ImageButton(name, static_cast<const ImTextureID>(image), size);
        }

        // raw ImGui backend API
        bool imgui_impl_init(const rl::Window& window);

        void imgui_impl_shutdown()
        {
            ImGuiIO& io{ ImGui::GetIO() };

            auto fontTexture{ static_cast<raylib::Texture2D*>(io.Fonts->TexID) };
            if (fontTexture && fontTexture->id != 0)
                raylib::UnloadTexture(*fontTexture);

            io.Fonts->TexID = nullptr;
        }

        void imgui_impl_render_draw_data(const ImDrawData* draw_data)
        {
            raylib::rlDrawRenderBatchActive();
            raylib::rlDisableBackfaceCulling();

            for (int l = 0; l < draw_data->CmdListsCount; ++l)
            {
                const ImDrawList* commandList{ draw_data->CmdLists[l] };
                for (const auto& cmd : commandList->CmdBuffer)
                {
                    this->EnableScissor(cmd.ClipRect.x - draw_data->DisplayPos.x,
                                        cmd.ClipRect.y - draw_data->DisplayPos.y,
                                        cmd.ClipRect.z - (cmd.ClipRect.x - draw_data->DisplayPos.x),
                                        cmd.ClipRect.w - (cmd.ClipRect.y - draw_data->DisplayPos.y));

                    if (cmd.UserCallback != nullptr)
                    {
                        cmd.UserCallback(commandList, &cmd);
                        continue;
                    }

                    this->imgui_render_triangles(cast::to<i32>(cmd.ElemCount),
                                                 cast::to<i32>(cmd.IdxOffset),
                                                 commandList->IdxBuffer,
                                                 commandList->VtxBuffer,
                                                 cmd.TextureId);

                    raylib::rlDrawRenderBatchActive();
                }
            }

            raylib::rlSetTexture(0);
            raylib::rlDisableScissorTest();
            raylib::rlEnableBackfaceCulling();
        }

        bool imgui_impl_process_events(const rl::Window& window);

    private:
        void reload_fonts_internal()
        {
            ImGuiIO& io{ ImGui::GetIO() };

            u8* pixels{ nullptr };
            size_t width{ 0 };
            size_t height{ 0 };
            io.Fonts->GetTexDataAsRGBA32(&pixels, reinterpret_cast<i32*>(&width),
                                         reinterpret_cast<i32*>(&height), nullptr);

            raylib::Image image{ raylib::GenImageColor(cast::to<i32>(width), cast::to<i32>(height),
                                                       raylib::BLANK) };
            std::memcpy(image.data, pixels, width * height * 4);

            raylib::Texture2D* font_texture{ static_cast<raylib::Texture2D*>(io.Fonts->TexID) };
            if (font_texture != nullptr && font_texture->id != 0)
            {
                raylib::UnloadTexture(*font_texture);
                delete font_texture;
                font_texture = nullptr;
            }

            runtime_assert(font_texture == nullptr, "leaking font texture, hasn't been deallocated");
            font_texture = new raylib::Texture2D{ raylib::LoadTextureFromImage(image) };
            raylib::UnloadImage(image);
            io.Fonts->TexID = font_texture;
        }

        void imgui_new_frame(const rl::Window& window, const rl::Display& display);

        void imgui_triangle_vert(ImDrawVert& idx_vert)
        {
            raylib::Color* c;
            c = reinterpret_cast<raylib::Color*>(&idx_vert.col);
            raylib::rlColor4ub(c->r, c->g, c->b, c->a);
            raylib::rlTexCoord2f(idx_vert.uv.x, idx_vert.uv.y);
            raylib::rlVertex2f(idx_vert.pos.x, idx_vert.pos.y);
        }

        void imgui_render_triangles(i32 count,
                                    i32 indexStart,
                                    const ImVector<ImDrawIdx>& indexBuffer,
                                    const ImVector<ImDrawVert>& vertBuffer,
                                    void* texture_data)
        {
            if (count < 3)
                return;

            raylib::Texture* texture = static_cast<raylib::Texture*>(texture_data);
            u32 textureId{ texture == nullptr ? 0 : texture->id };

            raylib::rlBegin(RL_TRIANGLES);
            raylib::rlSetTexture(textureId);

            for (i32 i = 0; i <= (count - 3); i += 3)
            {
                if (raylib::rlCheckRenderBatchLimit(3))
                {
                    raylib::rlBegin(RL_TRIANGLES);
                    raylib::rlSetTexture(textureId);
                }

                ImDrawIdx indexA{ indexBuffer[indexStart + i] };
                ImDrawIdx indexB{ indexBuffer[indexStart + i + 1] };
                ImDrawIdx indexC{ indexBuffer[indexStart + i + 2] };

                ImDrawVert vertexA{ vertBuffer[indexA] };
                ImDrawVert vertexB{ vertBuffer[indexB] };
                ImDrawVert vertexC{ vertBuffer[indexC] };

                this->imgui_triangle_vert(vertexA);
                this->imgui_triangle_vert(vertexB);
                this->imgui_triangle_vert(vertexC);
            }

            raylib::rlEnd();
        }

        void EnableScissor(float x, float y, float width, float height)
        {
            raylib::rlEnableScissorTest();
            ImGuiIO& io{ ImGui::GetIO() };
            raylib::rlScissor(cast::to<i32>(x * io.DisplayFramebufferScale.x),
                              cast::to<i32>((raylib::GetScreenHeight() - cast::to<i32>(y + height)) *
                                            cast::to<i32>(io.DisplayFramebufferScale.y)),
                              cast::to<i32>(width * io.DisplayFramebufferScale.x),
                              cast::to<i32>(height * io.DisplayFramebufferScale.y));
        }

        void setup_font_awesome()
        {
            static const ImWchar icons_ranges[] = {
                ICON_MIN_FA,
                ICON_MAX_FA,
                0,
            };

            ImFontConfig icons_config{};
            icons_config.MergeMode            = true;
            icons_config.PixelSnapH           = true;
            icons_config.FontDataOwnedByAtlas = false;
            icons_config.GlyphRanges          = icons_ranges;

            ImGuiIO& io{ ImGui::GetIO() };
            io.Fonts->AddFontFromMemoryCompressedTTF(
                reinterpret_cast<const void*>(fa_solid_900_compressed_data),
                fa_solid_900_compressed_size,
                gui::FONT_AWESOME_ICON_SIZE,
                &icons_config,
                icons_ranges);
        }

        void setup_imgui_backend()
        {
            ImGuiIO& io{ ImGui::GetIO() };
            io.BackendPlatformName = "imgui_impl_raylib";

            io.ClipboardUserData = nullptr;
            io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
            io.MousePos = { 0, 0 };

            io.SetClipboardTextFn = [](void*, const char* text) {
                return raylib::SetClipboardText(text);
            };

            io.GetClipboardTextFn = [](void*) {
                return raylib::GetClipboardText();
            };
        }

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
