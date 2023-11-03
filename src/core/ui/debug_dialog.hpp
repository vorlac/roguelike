#pragma once

#include <array>
#include <string>

#include <imgui.h>

#include "core/ui/imgui_helpers.hpp"

namespace rl::ui
{
    class debug_dialog
    {
    private:
        static constexpr bool* m_open{ nullptr };
        static inline bool m_use_work_area{ true };
        const ImGuiViewport* m_viewport{ ImGui::GetMainViewport() };
        static inline ImGuiWindowFlags m_flags = {
            ImGuiWindowFlags_NoDecoration |   //
            ImGuiWindowFlags_NoMove |         //
            ImGuiWindowFlags_NoSavedSettings  //
        };

    public:
        inline void update()
        {
            // We demonstrate using the full viewport area or the work area (without menu-bars,
            // task-bars etc.) Based on your use case you may want one or the other.
            ImGui::SetNextWindowPos(m_use_work_area ? m_viewport->WorkPos : m_viewport->Pos);
            ImGui::SetNextWindowSize(m_use_work_area ? m_viewport->WorkSize : m_viewport->Size);

            if (ImGui::Begin("Example: Fullscreen window", m_open, m_flags))
            {
                ImGui::Checkbox("Use work area instead of main area", &m_use_work_area);
                ImGui::SameLine();
                rl::ui::add_help_marker(
                    "Main Area = entire viewport,\n"
                    "Work Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\n"
                    "Enable the main-menu bar in Examples menu to see the difference.");

                // clang-format off
                ImGui::CheckboxFlags("NoBackground", &m_flags, ImGuiWindowFlags_NoBackground);
                ImGui::CheckboxFlags("NoDecoration", &m_flags, ImGuiWindowFlags_NoDecoration);
                ImGui::Indent();
                    ImGui::CheckboxFlags("NoTitleBar", &m_flags, ImGuiWindowFlags_NoTitleBar);
                    ImGui::CheckboxFlags("NoCollapse", &m_flags, ImGuiWindowFlags_NoCollapse);
                    ImGui::CheckboxFlags("NoScrollbar", &m_flags, ImGuiWindowFlags_NoScrollbar);
                ImGui::Unindent();
                // clang-format on

                if constexpr (m_open && ImGui::Button("Close this window"))
                    *m_open = false;
            }
            ImGui::End();
        }
    };
}
