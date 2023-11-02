#include <cmath>
#include <map>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "core/numeric_types.hpp"
#include "thirdparty/free_solid_font_data.hpp"
#include "thirdparty/raylib.hpp"
#include "thirdparty/rlimgui.hpp"

namespace rlimgui::internal
{
    static ImGuiMouseCursor CurrentMouseCursor = ImGuiMouseCursor_COUNT;
    static raylib::MouseCursor MouseCursorMap[ImGuiMouseCursor_COUNT];

    ImGuiContext* GlobalContext = nullptr;

    static std::map<raylib::KeyboardKey, ImGuiKey> RaylibKeyMap;

    static bool LastFrameFocused{ false };
    static bool LastControlPressed{ false };
    static bool LastShiftPressed{ false };
    static bool LastAltPressed{ false };
    static bool LastSuperPressed{ false };

    bool IsControlDown()
    {
        return raylib::IsKeyDown(raylib::KEY_RIGHT_CONTROL) ||
               raylib::IsKeyDown(raylib::KEY_LEFT_CONTROL);
    }

    bool IsShiftDown()
    {
        return raylib::IsKeyDown(raylib::KEY_RIGHT_SHIFT) ||
               raylib::IsKeyDown(raylib::KEY_LEFT_SHIFT);
    }

    bool IsAltDown()
    {
        return raylib::IsKeyDown(raylib::KEY_RIGHT_ALT) || raylib::IsKeyDown(raylib::KEY_LEFT_ALT);
    }

    bool IsSuperDown()
    {
        return raylib::IsKeyDown(raylib::KEY_RIGHT_SUPER) ||
               raylib::IsKeyDown(raylib::KEY_LEFT_SUPER);
    }

    void ReloadFonts()
    {
        ImGuiIO& io{ ImGui::GetIO() };
        unsigned char* pixels{ nullptr };

        int width{};
        int height{};

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, nullptr);
        raylib::Image image = raylib::GenImageColor(width, height, raylib::BLANK);
        memcpy(image.data, pixels, width * height * 4);

        raylib::Texture2D* fontTexture = (raylib::Texture2D*)io.Fonts->TexID;
        if (fontTexture && fontTexture->id != 0)
        {
            raylib::UnloadTexture(*fontTexture);
            raylib::MemFree(fontTexture);
        }

        fontTexture  = (raylib::Texture2D*)raylib::MemAlloc(sizeof(raylib::Texture2D));
        *fontTexture = raylib::LoadTextureFromImage(image);
        raylib::UnloadImage(image);
        io.Fonts->TexID = fontTexture;
    }

    static const char* GetClipTextCallback(void*)
    {
        return raylib::GetClipboardText();
    }

    static void SetClipTextCallback(void*, const char* text)
    {
        raylib::SetClipboardText(text);
    }

    static void ImGuiNewFrame(float deltaTime)
    {
        ImGuiIO& io{ ImGui::GetIO() };

        if (raylib::IsWindowFullscreen())
        {
            int monitor      = raylib::GetCurrentMonitor();
            io.DisplaySize.x = float(raylib::GetMonitorWidth(monitor));
            io.DisplaySize.y = float(raylib::GetMonitorHeight(monitor));
        }
        else
        {
            io.DisplaySize.x = float(raylib::GetScreenWidth());
            io.DisplaySize.y = float(raylib::GetScreenHeight());
        }

        int width = int(io.DisplaySize.x), height = int(io.DisplaySize.y);
        glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

        if (width > 0 && height > 0)
            io.DisplayFramebufferScale = ImVec2(width / io.DisplaySize.x, height / io.DisplaySize.y);
        else
            io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        io.DeltaTime = deltaTime;

        if (io.WantSetMousePos)
        {
            raylib::SetMousePosition(static_cast<rl::i32>(io.MousePos.x),
                                     static_cast<rl::i32>(io.MousePos.y));
        }
        else
        {
            io.MousePos.x = static_cast<float>(raylib::GetMouseX());
            io.MousePos.y = static_cast<float>(raylib::GetMouseY());
        }

        io.MouseDown[0] = raylib::IsMouseButtonDown(raylib::MOUSE_LEFT_BUTTON);
        io.MouseDown[1] = raylib::IsMouseButtonDown(raylib::MOUSE_RIGHT_BUTTON);
        io.MouseDown[2] = raylib::IsMouseButtonDown(raylib::MOUSE_MIDDLE_BUTTON);

        {
            raylib::Vector2 mouseWheel{ raylib::GetMouseWheelMoveV() };
            io.MouseWheel += mouseWheel.y;
            io.MouseWheelH += mouseWheel.x;
        }

        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
        {
            ImGuiMouseCursor imgui_cursor{ ImGui::GetMouseCursor() };
            if (imgui_cursor != internal::CurrentMouseCursor || io.MouseDrawCursor)
            {
                internal::CurrentMouseCursor = imgui_cursor;
                if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
                {
                    raylib::HideCursor();
                }
                else
                {
                    raylib::ShowCursor();

                    if (!(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
                    {
                        raylib::SetMouseCursor(
                            (imgui_cursor > -1 && imgui_cursor < ImGuiMouseCursor_COUNT)
                                ? MouseCursorMap[imgui_cursor]
                                : raylib::MOUSE_CURSOR_DEFAULT);
                    }
                }
            }
        }
    }

    static void ImGuiTriangleVert(ImDrawVert& idx_vert)
    {
        raylib::Color* c;
        c = reinterpret_cast<raylib::Color*>(&idx_vert.col);
        raylib::rlColor4ub(c->r, c->g, c->b, c->a);
        raylib::rlTexCoord2f(idx_vert.uv.x, idx_vert.uv.y);
        raylib::rlVertex2f(idx_vert.pos.x, idx_vert.pos.y);
    }

    static void ImGuiRenderTriangles(rl::i32 count,
                                     rl::i32 indexStart,
                                     const ImVector<ImDrawIdx>& indexBuffer,
                                     const ImVector<ImDrawVert>& vertBuffer,
                                     void* texturePtr)
    {
        if (count < 3)
            return;

        raylib::Texture* texture = static_cast<raylib::Texture*>(texturePtr);

        unsigned int textureId{ texture == nullptr ? 0 : texture->id };

        raylib::rlBegin(RL_TRIANGLES);
        raylib::rlSetTexture(textureId);

        for (rl::i32 i = 0; i <= (count - 3); i += 3)
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

            ImGuiTriangleVert(vertexA);
            ImGuiTriangleVert(vertexB);
            ImGuiTriangleVert(vertexC);
        }

        raylib::rlEnd();
    }

    static void EnableScissor(float x, float y, float width, float height)
    {
        raylib::rlEnableScissorTest();
        ImGuiIO& io{ ImGui::GetIO() };
        raylib::rlScissor(
            static_cast<rl::i32>(x * io.DisplayFramebufferScale.x),
            static_cast<rl::i32>((raylib::GetScreenHeight() - static_cast<rl::i32>(y + height)) *
                                 io.DisplayFramebufferScale.y),
            static_cast<rl::i32>(width * io.DisplayFramebufferScale.x),
            static_cast<rl::i32>(height * io.DisplayFramebufferScale.y));
    }

    static void SetupMouseCursors()
    {
        internal::MouseCursorMap[ImGuiMouseCursor_Arrow]      = raylib::MOUSE_CURSOR_ARROW;
        internal::MouseCursorMap[ImGuiMouseCursor_TextInput]  = raylib::MOUSE_CURSOR_IBEAM;
        internal::MouseCursorMap[ImGuiMouseCursor_Hand]       = raylib::MOUSE_CURSOR_POINTING_HAND;
        internal::MouseCursorMap[ImGuiMouseCursor_ResizeAll]  = raylib::MOUSE_CURSOR_RESIZE_ALL;
        internal::MouseCursorMap[ImGuiMouseCursor_ResizeEW]   = raylib::MOUSE_CURSOR_RESIZE_EW;
        internal::MouseCursorMap[ImGuiMouseCursor_ResizeNESW] = raylib::MOUSE_CURSOR_RESIZE_NESW;
        internal::MouseCursorMap[ImGuiMouseCursor_ResizeNS]   = raylib::MOUSE_CURSOR_RESIZE_NS;
        internal::MouseCursorMap[ImGuiMouseCursor_ResizeNWSE] = raylib::MOUSE_CURSOR_RESIZE_NWSE;
        internal::MouseCursorMap[ImGuiMouseCursor_NotAllowed] = raylib::MOUSE_CURSOR_NOT_ALLOWED;
    }

    void SetupFontAwesome()
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
        io.Fonts->AddFontFromMemoryCompressedTTF((void*)(fa_solid_900_compressed_data),
                                                 fa_solid_900_compressed_size,
                                                 FONT_AWESOME_ICON_SIZE,
                                                 &icons_config,
                                                 icons_ranges);
    }

    void SetupBackend()
    {
        ImGuiIO& io{ ImGui::GetIO() };
        io.BackendPlatformName = "imgui_impl_raylib";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.MousePos           = { 0, 0 };
        io.SetClipboardTextFn = internal::SetClipTextCallback;
        io.GetClipboardTextFn = internal::GetClipTextCallback;
        io.ClipboardUserData  = nullptr;
    }

    static void SetupKeymap()
    {
        if (!internal::RaylibKeyMap.empty())
            return;

        // build up a map of raylib keys to ImGuiKeys
        internal::RaylibKeyMap[raylib::KEY_APOSTROPHE]    = ImGuiKey_Apostrophe;
        internal::RaylibKeyMap[raylib::KEY_COMMA]         = ImGuiKey_Comma;
        internal::RaylibKeyMap[raylib::KEY_MINUS]         = ImGuiKey_Minus;
        internal::RaylibKeyMap[raylib::KEY_PERIOD]        = ImGuiKey_Period;
        internal::RaylibKeyMap[raylib::KEY_SLASH]         = ImGuiKey_Slash;
        internal::RaylibKeyMap[raylib::KEY_ZERO]          = ImGuiKey_0;
        internal::RaylibKeyMap[raylib::KEY_ONE]           = ImGuiKey_1;
        internal::RaylibKeyMap[raylib::KEY_TWO]           = ImGuiKey_2;
        internal::RaylibKeyMap[raylib::KEY_THREE]         = ImGuiKey_3;
        internal::RaylibKeyMap[raylib::KEY_FOUR]          = ImGuiKey_4;
        internal::RaylibKeyMap[raylib::KEY_FIVE]          = ImGuiKey_5;
        internal::RaylibKeyMap[raylib::KEY_SIX]           = ImGuiKey_6;
        internal::RaylibKeyMap[raylib::KEY_SEVEN]         = ImGuiKey_7;
        internal::RaylibKeyMap[raylib::KEY_EIGHT]         = ImGuiKey_8;
        internal::RaylibKeyMap[raylib::KEY_NINE]          = ImGuiKey_9;
        internal::RaylibKeyMap[raylib::KEY_SEMICOLON]     = ImGuiKey_Semicolon;
        internal::RaylibKeyMap[raylib::KEY_EQUAL]         = ImGuiKey_Equal;
        internal::RaylibKeyMap[raylib::KEY_A]             = ImGuiKey_A;
        internal::RaylibKeyMap[raylib::KEY_B]             = ImGuiKey_B;
        internal::RaylibKeyMap[raylib::KEY_C]             = ImGuiKey_C;
        internal::RaylibKeyMap[raylib::KEY_D]             = ImGuiKey_D;
        internal::RaylibKeyMap[raylib::KEY_E]             = ImGuiKey_E;
        internal::RaylibKeyMap[raylib::KEY_F]             = ImGuiKey_F;
        internal::RaylibKeyMap[raylib::KEY_G]             = ImGuiKey_G;
        internal::RaylibKeyMap[raylib::KEY_H]             = ImGuiKey_H;
        internal::RaylibKeyMap[raylib::KEY_I]             = ImGuiKey_I;
        internal::RaylibKeyMap[raylib::KEY_J]             = ImGuiKey_J;
        internal::RaylibKeyMap[raylib::KEY_K]             = ImGuiKey_K;
        internal::RaylibKeyMap[raylib::KEY_L]             = ImGuiKey_L;
        internal::RaylibKeyMap[raylib::KEY_M]             = ImGuiKey_M;
        internal::RaylibKeyMap[raylib::KEY_N]             = ImGuiKey_N;
        internal::RaylibKeyMap[raylib::KEY_O]             = ImGuiKey_O;
        internal::RaylibKeyMap[raylib::KEY_P]             = ImGuiKey_P;
        internal::RaylibKeyMap[raylib::KEY_Q]             = ImGuiKey_Q;
        internal::RaylibKeyMap[raylib::KEY_R]             = ImGuiKey_R;
        internal::RaylibKeyMap[raylib::KEY_S]             = ImGuiKey_S;
        internal::RaylibKeyMap[raylib::KEY_T]             = ImGuiKey_T;
        internal::RaylibKeyMap[raylib::KEY_U]             = ImGuiKey_U;
        internal::RaylibKeyMap[raylib::KEY_V]             = ImGuiKey_V;
        internal::RaylibKeyMap[raylib::KEY_W]             = ImGuiKey_W;
        internal::RaylibKeyMap[raylib::KEY_X]             = ImGuiKey_X;
        internal::RaylibKeyMap[raylib::KEY_Y]             = ImGuiKey_Y;
        internal::RaylibKeyMap[raylib::KEY_Z]             = ImGuiKey_Z;
        internal::RaylibKeyMap[raylib::KEY_SPACE]         = ImGuiKey_Space;
        internal::RaylibKeyMap[raylib::KEY_ESCAPE]        = ImGuiKey_Escape;
        internal::RaylibKeyMap[raylib::KEY_ENTER]         = ImGuiKey_Enter;
        internal::RaylibKeyMap[raylib::KEY_TAB]           = ImGuiKey_Tab;
        internal::RaylibKeyMap[raylib::KEY_BACKSPACE]     = ImGuiKey_Backspace;
        internal::RaylibKeyMap[raylib::KEY_INSERT]        = ImGuiKey_Insert;
        internal::RaylibKeyMap[raylib::KEY_DELETE]        = ImGuiKey_Delete;
        internal::RaylibKeyMap[raylib::KEY_RIGHT]         = ImGuiKey_RightArrow;
        internal::RaylibKeyMap[raylib::KEY_LEFT]          = ImGuiKey_LeftArrow;
        internal::RaylibKeyMap[raylib::KEY_DOWN]          = ImGuiKey_DownArrow;
        internal::RaylibKeyMap[raylib::KEY_UP]            = ImGuiKey_UpArrow;
        internal::RaylibKeyMap[raylib::KEY_PAGE_UP]       = ImGuiKey_PageUp;
        internal::RaylibKeyMap[raylib::KEY_PAGE_DOWN]     = ImGuiKey_PageDown;
        internal::RaylibKeyMap[raylib::KEY_HOME]          = ImGuiKey_Home;
        internal::RaylibKeyMap[raylib::KEY_END]           = ImGuiKey_End;
        internal::RaylibKeyMap[raylib::KEY_CAPS_LOCK]     = ImGuiKey_CapsLock;
        internal::RaylibKeyMap[raylib::KEY_SCROLL_LOCK]   = ImGuiKey_ScrollLock;
        internal::RaylibKeyMap[raylib::KEY_NUM_LOCK]      = ImGuiKey_NumLock;
        internal::RaylibKeyMap[raylib::KEY_PRINT_SCREEN]  = ImGuiKey_PrintScreen;
        internal::RaylibKeyMap[raylib::KEY_PAUSE]         = ImGuiKey_Pause;
        internal::RaylibKeyMap[raylib::KEY_F1]            = ImGuiKey_F1;
        internal::RaylibKeyMap[raylib::KEY_F2]            = ImGuiKey_F2;
        internal::RaylibKeyMap[raylib::KEY_F3]            = ImGuiKey_F3;
        internal::RaylibKeyMap[raylib::KEY_F4]            = ImGuiKey_F4;
        internal::RaylibKeyMap[raylib::KEY_F5]            = ImGuiKey_F5;
        internal::RaylibKeyMap[raylib::KEY_F6]            = ImGuiKey_F6;
        internal::RaylibKeyMap[raylib::KEY_F7]            = ImGuiKey_F7;
        internal::RaylibKeyMap[raylib::KEY_F8]            = ImGuiKey_F8;
        internal::RaylibKeyMap[raylib::KEY_F9]            = ImGuiKey_F9;
        internal::RaylibKeyMap[raylib::KEY_F10]           = ImGuiKey_F10;
        internal::RaylibKeyMap[raylib::KEY_F11]           = ImGuiKey_F11;
        internal::RaylibKeyMap[raylib::KEY_F12]           = ImGuiKey_F12;
        internal::RaylibKeyMap[raylib::KEY_LEFT_SHIFT]    = ImGuiKey_LeftShift;
        internal::RaylibKeyMap[raylib::KEY_LEFT_CONTROL]  = ImGuiKey_LeftCtrl;
        internal::RaylibKeyMap[raylib::KEY_LEFT_ALT]      = ImGuiKey_LeftAlt;
        internal::RaylibKeyMap[raylib::KEY_LEFT_SUPER]    = ImGuiKey_LeftSuper;
        internal::RaylibKeyMap[raylib::KEY_RIGHT_SHIFT]   = ImGuiKey_RightShift;
        internal::RaylibKeyMap[raylib::KEY_RIGHT_CONTROL] = ImGuiKey_RightCtrl;
        internal::RaylibKeyMap[raylib::KEY_RIGHT_ALT]     = ImGuiKey_RightAlt;
        internal::RaylibKeyMap[raylib::KEY_RIGHT_SUPER]   = ImGuiKey_RightSuper;
        internal::RaylibKeyMap[raylib::KEY_KB_MENU]       = ImGuiKey_Menu;
        internal::RaylibKeyMap[raylib::KEY_LEFT_BRACKET]  = ImGuiKey_LeftBracket;
        internal::RaylibKeyMap[raylib::KEY_BACKSLASH]     = ImGuiKey_Backslash;
        internal::RaylibKeyMap[raylib::KEY_RIGHT_BRACKET] = ImGuiKey_RightBracket;
        internal::RaylibKeyMap[raylib::KEY_GRAVE]         = ImGuiKey_GraveAccent;
        internal::RaylibKeyMap[raylib::KEY_KP_0]          = ImGuiKey_Keypad0;
        internal::RaylibKeyMap[raylib::KEY_KP_1]          = ImGuiKey_Keypad1;
        internal::RaylibKeyMap[raylib::KEY_KP_2]          = ImGuiKey_Keypad2;
        internal::RaylibKeyMap[raylib::KEY_KP_3]          = ImGuiKey_Keypad3;
        internal::RaylibKeyMap[raylib::KEY_KP_4]          = ImGuiKey_Keypad4;
        internal::RaylibKeyMap[raylib::KEY_KP_5]          = ImGuiKey_Keypad5;
        internal::RaylibKeyMap[raylib::KEY_KP_6]          = ImGuiKey_Keypad6;
        internal::RaylibKeyMap[raylib::KEY_KP_7]          = ImGuiKey_Keypad7;
        internal::RaylibKeyMap[raylib::KEY_KP_8]          = ImGuiKey_Keypad8;
        internal::RaylibKeyMap[raylib::KEY_KP_9]          = ImGuiKey_Keypad9;
        internal::RaylibKeyMap[raylib::KEY_KP_DECIMAL]    = ImGuiKey_KeypadDecimal;
        internal::RaylibKeyMap[raylib::KEY_KP_DIVIDE]     = ImGuiKey_KeypadDivide;
        internal::RaylibKeyMap[raylib::KEY_KP_MULTIPLY]   = ImGuiKey_KeypadMultiply;
        internal::RaylibKeyMap[raylib::KEY_KP_SUBTRACT]   = ImGuiKey_KeypadSubtract;
        internal::RaylibKeyMap[raylib::KEY_KP_ADD]        = ImGuiKey_KeypadAdd;
        internal::RaylibKeyMap[raylib::KEY_KP_ENTER]      = ImGuiKey_KeypadEnter;
        internal::RaylibKeyMap[raylib::KEY_KP_EQUAL]      = ImGuiKey_KeypadEqual;
    }

    static void SetupGlobals()
    {
        internal::LastFrameFocused   = raylib::IsWindowFocused();
        internal::LastControlPressed = false;
        internal::LastShiftPressed   = false;
        internal::LastAltPressed     = false;
        internal::LastSuperPressed   = false;
    }
}

namespace rlimgui
{
    void EndInitImGui()
    {
        ImGui::SetCurrentContext(internal::GlobalContext);
        internal::SetupFontAwesome();
        internal::SetupMouseCursors();
        internal::SetupBackend();
        internal::ReloadFonts();
    }

    void BeginInitImGui()
    {
        internal::SetupGlobals();
        internal::GlobalContext = ImGui::CreateContext(nullptr);
        internal::SetupKeymap();

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontDefault();
    }

    void Setup(bool dark)
    {
        BeginInitImGui();

        if (dark)
            ImGui::StyleColorsDark();
        else
            ImGui::StyleColorsLight();

        EndInitImGui();
    }

    void ReloadFonts()
    {
        ImGui::SetCurrentContext(internal::GlobalContext);
        internal::ReloadFonts();
    }

    void Begin()
    {
        ImGui::SetCurrentContext(internal::GlobalContext);
        BeginDelta(raylib::GetFrameTime());
    }

    void Image(const raylib::Texture* image)
    {
        if (!image)
            return;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);
        ImGui::Image((ImTextureID)image, ImVec2(float(image->width), float(image->height)));
    }

    bool ImageButton(const char* name, const raylib::Texture* image)
    {
        if (!image)
            return false;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);
        return ImGui::ImageButton(name,
                                  (ImTextureID)image,
                                  ImVec2(float(image->width), float(image->height)));
    }

    bool ImageButtonSize(const char* name, const raylib::Texture* image, ImVec2 size)
    {
        if (!image)
            return false;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);
        return ImGui::ImageButton(name, (ImTextureID)image, size);
    }

    void ImageSize(const raylib::Texture* image, int width, int height)
    {
        if (!image)
            return;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);
        ImGui::Image((ImTextureID)image, ImVec2(float(width), float(height)));
    }

    void ImageSizeV(const raylib::Texture* image, raylib::Vector2 size)
    {
        if (!image)
            return;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);
        ImGui::Image((ImTextureID)image, ImVec2(size.x, size.y));
    }

    void ImageRect(const raylib::Texture* image,
                   int destWidth,
                   int destHeight,
                   raylib::Rectangle sourceRect)
    {
        if (!image)
            return;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);

        ImVec2 uv0;
        ImVec2 uv1;

        if (sourceRect.width < 0)
        {
            uv0.x = -((float)sourceRect.x / image->width);
            uv1.x = (uv0.x - static_cast<float>(std::fabs(sourceRect.width) / image->width));
        }
        else
        {
            uv0.x = static_cast<float>(sourceRect.x) / image->width;
            uv1.x = uv0.x + static_cast<float>(sourceRect.width / image->width);
        }

        if (sourceRect.height < 0)
        {
            uv0.y = -static_cast<float>(sourceRect.y) / image->height;
            uv1.y = (uv0.y - (float)(fabs(sourceRect.height) / image->height));
        }
        else
        {
            uv0.y = static_cast<float>(sourceRect.y) / image->height;
            uv1.y = uv0.y + (float)(sourceRect.height / image->height);
        }

        ImGui::Image((ImTextureID)image, ImVec2(float(destWidth), float(destHeight)), uv0, uv1);
    }

    // API
    void ImageRenderTexture(const raylib::RenderTexture* image)
    {
        if (!image)
            return;

        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);

        ImageRect(
            &image->texture,
            image->texture.width,
            image->texture.height,
            raylib::Rectangle{ 0, 0, float(image->texture.width), -float(image->texture.height) });
    }

    // API
    void ImageRenderTextureFit(const raylib::RenderTexture* image, bool center)
    {
        if (!image)
            return;
        if (internal::GlobalContext)
            ImGui::SetCurrentContext(internal::GlobalContext);

        ImVec2 area = ImGui::GetContentRegionAvail();

        float scale = area.x / image->texture.width;

        float y = image->texture.height * scale;
        if (y > area.y)
            scale = area.y / image->texture.height;

        int sizeX = int(image->texture.width * scale);
        int sizeY = int(image->texture.height * scale);

        if (center)
        {
            ImGui::SetCursorPosX(0);
            ImGui::SetCursorPosX(area.x / 2 - sizeX / 2);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (area.y / 2 - sizeY / 2));
        }

        ImageRect(
            &image->texture,
            sizeX,
            sizeY,
            raylib::Rectangle{ 0, 0, float(image->texture.width), -float(image->texture.height) });
    }

    // raw ImGui backend API
    bool ImGui_ImplRaylib_Init()
    {
        internal::SetupGlobals();
        internal::SetupKeymap();

        ImGuiIO& io{ ImGui::GetIO() };
        io.Fonts->AddFontDefault();

        internal::SetupFontAwesome();
        internal::SetupMouseCursors();
        internal::SetupBackend();
        internal::ReloadFonts();

        return true;
    }

    void ImGui_ImplRaylib_Shutdown()
    {
        ImGuiIO& io{ ImGui::GetIO() };
        raylib::Texture2D* fontTexture = (raylib::Texture2D*)io.Fonts->TexID;

        if (fontTexture && fontTexture->id != 0)
            raylib::UnloadTexture(*fontTexture);

        io.Fonts->TexID = 0;
    }

    void ImGui_ImplRaylib_NewFrame()
    {
        internal::ImGuiNewFrame(raylib::GetFrameTime());
    }

    void ImGui_ImplRaylib_RenderDrawData(ImDrawData* draw_data)
    {
        raylib::rlDrawRenderBatchActive();
        raylib::rlDisableBackfaceCulling();

        for (int l = 0; l < draw_data->CmdListsCount; ++l)
        {
            const ImDrawList* commandList = draw_data->CmdLists[l];

            for (const auto& cmd : commandList->CmdBuffer)
            {
                internal::EnableScissor(cmd.ClipRect.x - draw_data->DisplayPos.x,
                                        cmd.ClipRect.y - draw_data->DisplayPos.y,
                                        cmd.ClipRect.z - (cmd.ClipRect.x - draw_data->DisplayPos.x),
                                        cmd.ClipRect.w - (cmd.ClipRect.y - draw_data->DisplayPos.y));

                if (cmd.UserCallback != nullptr)
                {
                    cmd.UserCallback(commandList, &cmd);
                    continue;
                }

                internal::ImGuiRenderTriangles(cmd.ElemCount,
                                               cmd.IdxOffset,
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

    bool ImGui_ImplRaylib_ProcessEvents()
    {
        ImGuiIO& io{ ImGui::GetIO() };

        bool focused = raylib::IsWindowFocused();
        if (focused != internal::LastFrameFocused)
            io.AddFocusEvent(focused);
        internal::LastFrameFocused = focused;

        // handle the modifyer key events so that shortcuts work
        bool ctrlDown = internal::IsControlDown();
        if (ctrlDown != internal::LastControlPressed)
            io.AddKeyEvent(ImGuiMod_Ctrl, ctrlDown);
        internal::LastControlPressed = ctrlDown;

        bool shiftDown = internal::IsShiftDown();
        if (shiftDown != internal::LastShiftPressed)
            io.AddKeyEvent(ImGuiMod_Shift, shiftDown);
        internal::LastShiftPressed = shiftDown;

        bool altDown = internal::IsAltDown();
        if (altDown != internal::LastAltPressed)
            io.AddKeyEvent(ImGuiMod_Alt, altDown);
        internal::LastAltPressed = altDown;

        bool superDown = internal::IsSuperDown();
        if (superDown != internal::LastSuperPressed)
            io.AddKeyEvent(ImGuiMod_Super, superDown);
        internal::LastSuperPressed = superDown;

        // get the pressed keys, they are in event order
        int keyId = raylib::GetKeyPressed();
        while (keyId != 0)
        {
            auto keyItr = internal::RaylibKeyMap.find(raylib::KeyboardKey(keyId));
            if (keyItr != internal::RaylibKeyMap.end())
                io.AddKeyEvent(keyItr->second, true);
            keyId = raylib::GetKeyPressed();
        }

        // look for any keys that were down last frame and see if they were down and are released
        for (const auto keyItr : internal::RaylibKeyMap)
            if (raylib::IsKeyReleased(keyItr.first))
                io.AddKeyEvent(keyItr.second, false);

        // add the text input in order
        unsigned int pressed = raylib::GetCharPressed();
        while (pressed != 0)
        {
            io.AddInputCharacter(pressed);
            pressed = raylib::GetCharPressed();
        }

        return true;
    }

    // API
    void BeginDelta(float deltaTime)
    {
        ImGui::SetCurrentContext(internal::GlobalContext);
        internal::ImGuiNewFrame(deltaTime);
        ImGui_ImplRaylib_ProcessEvents();
        ImGui::NewFrame();
    }

    // API
    void End()
    {
        ImGui::SetCurrentContext(internal::GlobalContext);
        ImGui::Render();
        ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
    }

    // API
    void Shutdown()
    {
        ImGui::SetCurrentContext(internal::GlobalContext);
        ImGui_ImplRaylib_Shutdown();
        ImGui::DestroyContext();
    }
}
