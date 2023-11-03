#pragma once

#include <functional>

#include <imgui.h>

namespace rl::ui
{
    // typedef void (*ImGuiDemoMarkerCallback)(const i8*, i32, const i8*, void*);
    // const char* file, int line, const char* section, void* user_data
    using event_callback = std::function<void(std::string, i32, std::string, void*)>;

    /**
     * @brief Appends a `(?)` marker symbol to the UI being created
     * along with tooltip text that appears on mouse cursor hover
     * */
    inline void add_help_marker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

}
