#pragma once

#include <imgui/imgui.h>

namespace rl::ui
{
    void update()
    {
        ImGuiWindowFlags window_flags{ ImGuiWindowFlags_None };
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_MenuBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoNav;
        window_flags |= ImGuiWindowFlags_NoBackground;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        window_flags |= ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_UnsavedDocument;
        p_open = nullptr;  // Don't pass our bool* to Begin

        if (!ImGui::Begin("debug", nullptr, window_flags))
        {
            // window is collapsed
            ImGui::End();
            return;
        }
    }
}
