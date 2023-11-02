#include <atomic>
#include <vector>

#include <GLFW/glfw3.h>

#include "core/display.hpp"
#include "core/gui.hpp"
#include "core/window.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    void gui::pre_init(const rl::Window& window)
    {
        this->reset_gui_state(window);
        m_context = ImGui::CreateContext();
        ImGuiIO& io{ ImGui::GetIO() };
        io.Fonts->AddFontDefault();
    }

    void gui::init(const rl::Window& window, bool dark_theme /* = true*/)
    {
        this->pre_init(window);
        dark_theme ? ImGui::StyleColorsDark()  //
                   : ImGui::StyleColorsLight();
        this->post_init();
    }

    void gui::post_init()
    {
        ImGui::SetCurrentContext(m_context);
        this->setup_font_awesome();
        this->setup_imgui_backend();
        this->imgui_reload_fonts();
    }

    void gui::begin(const rl::Window& window, const rl::Display& display)
    {
        ImGui::SetCurrentContext(m_context);
        this->begin_delta(window, display);
    }

    void gui::begin_delta(const rl::Window& window, const rl::Display& display)
    {
        ImGui::SetCurrentContext(m_context);
        this->imgui_new_frame(window, display);
        this->imgui_process_events(window);
        ImGui::NewFrame();
    }

    void gui::update(const Window& window, const Display& display)
    {
        this->begin(window, display);
        bool open = true;
        ImGui::ShowDemoWindow(&open);
        this->end();
    }

    void gui::reload_fonts()
    {
        ImGui::SetCurrentContext(m_context);
        this->imgui_reload_fonts();
    }

    void gui::end()
    {
        ImGui::SetCurrentContext(m_context);
        ImGui::Render();

        const ImDrawData* draw_data{ ImGui::GetDrawData() };
        this->imgui_render(draw_data);
    }

    void gui::teardown()
    {
        ImGui::SetCurrentContext(m_context);
        this->imgui_shutdown();
        ImGui::DestroyContext();
    }

    void gui::imgui_process_events(const rl::Window& window)
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

    bool gui::imgui_init(const rl::Window& window)
    {
        this->reset_gui_state(window);

        ImGuiIO& io{ ImGui::GetIO() };
        io.Fonts->AddFontDefault();

        this->setup_font_awesome();
        this->setup_imgui_backend();
        this->imgui_reload_fonts();

        return true;
    }

    void gui::reset_gui_state(const rl::Window& window)
    {
        m_last_frame_focused   = window.is_focused();
        m_last_control_pressed = false;
        m_last_shift_pressed   = false;
        m_last_alt_pressed     = false;
        m_last_super_pressed   = false;
    }

    void gui::enable_scissor(float x, float y, float width, float height)
    {
        raylib::rlEnableScissorTest();
        ImGuiIO& io{ ImGui::GetIO() };
        raylib::rlScissor(cast::to<i32>(x * io.DisplayFramebufferScale.x),
                          cast::to<i32>((raylib::GetScreenHeight() - cast::to<i32>(y + height)) *
                                        cast::to<i32>(io.DisplayFramebufferScale.y)),
                          cast::to<i32>(width * io.DisplayFramebufferScale.x),
                          cast::to<i32>(height * io.DisplayFramebufferScale.y));
    }

    bool gui::image_button(const char* name, raylib::Texture* image)
    {
        if (image == nullptr)
            return false;
        if (m_context)
            ImGui::SetCurrentContext(m_context);

        return ImGui::ImageButton(name,
                                  static_cast<const ImTextureID>(image),
                                  ImVec2(cast::to<f32>(image->width), cast::to<f32>(image->height)));
    }

    bool gui::image_button_size(const char* name, raylib::Texture* image, ImVec2 size)
    {
        if (image == nullptr)
            return false;
        if (m_context != nullptr)
            ImGui::SetCurrentContext(m_context);

        return ImGui::ImageButton(name, static_cast<const ImTextureID>(image), size);
    }

    void gui::imgui_render(const ImDrawData* draw_data)
    {
        raylib::rlDrawRenderBatchActive();
        raylib::rlDisableBackfaceCulling();

        for (int l = 0; l < draw_data->CmdListsCount; ++l)
        {
            const ImDrawList* commandList{ draw_data->CmdLists[l] };
            for (const auto& cmd : commandList->CmdBuffer)
            {
                this->enable_scissor(cmd.ClipRect.x - draw_data->DisplayPos.x,
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

    void gui::image(raylib::Texture* image)
    {
        if (image == nullptr)
            return;
        if (m_context != nullptr)
            ImGui::SetCurrentContext(m_context);

        ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                     ImVec2(cast::to<f32>(image->width), cast::to<f32>(image->height)));
    }

    void gui::image_size(raylib::Texture* image, i32 width, i32 height)
    {
        if (image == nullptr)
            return;
        if (m_context != nullptr)
            ImGui::SetCurrentContext(m_context);

        ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                     ImVec2(cast::to<f32>(width), cast::to<f32>(height)));
    }

    void gui::image_rect(raylib::Texture* image, i32 destWidth, i32 destHeight,
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
    }

    void gui::image_size_v(raylib::Texture* image, raylib::Vector2 size)
    {
        if (image == nullptr)
            return;
        if (m_context != nullptr)
            ImGui::SetCurrentContext(m_context);

        ImGui::Image(reinterpret_cast<const ImTextureID>(image),
                     ImVec2(cast::to<f32>(size.x), cast::to<f32>(size.y)));
    }

    void gui::image_render_texture(raylib::RenderTexture* image)
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

    void gui::image_render_texture_fit(raylib::RenderTexture* image, bool center)
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

    void gui::imgui_triangle_vert(ImDrawVert& idx_vert)
    {
        raylib::Color* c;
        c = reinterpret_cast<raylib::Color*>(&idx_vert.col);
        raylib::rlColor4ub(c->r, c->g, c->b, c->a);
        raylib::rlTexCoord2f(idx_vert.uv.x, idx_vert.uv.y);
        raylib::rlVertex2f(idx_vert.pos.x, idx_vert.pos.y);
    }

    void gui::imgui_render_triangles(i32 count,
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

    void gui::imgui_reload_fonts()
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

    void gui::setup_font_awesome()
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

    void gui::setup_imgui_backend()
    {
        ImGuiIO& io{ ImGui::GetIO() };
        io.BackendPlatformName = "imgui_raylib";

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

    void gui::imgui_shutdown()
    {
        ImGuiIO& io{ ImGui::GetIO() };

        auto fontTexture{ static_cast<raylib::Texture2D*>(io.Fonts->TexID) };
        if (fontTexture && fontTexture->id != 0)
            raylib::UnloadTexture(*fontTexture);

        io.Fonts->TexID = nullptr;
    }

}
