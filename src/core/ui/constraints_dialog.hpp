#pragma once

#include <array>
#include <string>
#include <utility>

#include <imgui.h>

#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/conversions.hpp"

namespace rl::ui
{
    class constraints_dialog
    {
    private:
        bool m_auto_resize{ false };
        bool m_window_padding{ true };
        i32 m_display_lines{ 10 };
        i32 m_type{ 5 };

        static inline constexpr bool* m_open{ nullptr };
        static inline constexpr std::array m_test_desc{
            "Between 100x100 and 500x500",  //
            "At least 100x100",             //
            "Resize vertical only",         //
            "Resize horizontal only",       //
            "Width Between 400 and 500",    //
            "Custom: Aspect Ratio 16:9",    //
            "Custom: Always Square",        //
            "Custom: Fixed Steps (100)",    //
        };

    private:
        struct resize_contraints
        {
            static void aspect_ratio(ImGuiSizeCallbackData* data)
            {
                f32 aspect_ratio{ *static_cast<f32*>(data->UserData) };
                data->DesiredSize.x = std::max(data->CurrentSize.x, data->CurrentSize.y);
                data->DesiredSize.y = cast::to<f32>(cast::to<i32>(data->DesiredSize.x / aspect_ratio));
            }

            static void square(ImGuiSizeCallbackData* data)

            {
                data->DesiredSize.x = data->DesiredSize.y = std::max(data->CurrentSize.x,
                                                                     data->CurrentSize.y);
            }

            static void step(ImGuiSizeCallbackData* data)
            {
                f32 step{ *static_cast<f32*>(data->UserData) };
                data->DesiredSize = {
                    cast::to<i32>(data->CurrentSize.x / step + 0.5f) * step,
                    cast::to<i32>(data->CurrentSize.y / step + 0.5f) * step,
                };
            }
        };

    public:
        inline void update()
        {
            static constexpr auto apply_constraint =
                [&](ImVec2&& min, ImVec2&& max, ImGuiSizeCallback custom_callback = nullptr,
                    void* custom_callback_data = nullptr) {  // wrappper lambda to keep the code
                                                             // below cleaerer / cleaner
                    ImGui::SetNextWindowSizeConstraints(min, max, custom_callback,
                                                        custom_callback_data);
                };

            // Submit constraint
            f32 aspect_ratio{ 16.0f / 9.0f };
            f32 fixed_step{ 100.0f };

            switch (m_type)
            {
                case 0:
                    // Between 100x100 and 500x500
                    apply_constraint({ 100, 100 }, { 500, 500 });
                    break;

                case 1:
                    // Width > 100,  Height > 100
                    apply_constraint({ 100, 100 }, { FLT_MAX, FLT_MAX });
                    break;

                case 2:
                    // Vertical  only
                    apply_constraint({ -1, 0 }, { -1, FLT_MAX });
                    break;

                case 3:
                    // Horizontal  only
                    apply_constraint({ 0, -1 }, { FLT_MAX, -1 });
                    break;

                case 4:
                    // Width Between and  400 and 500
                    apply_constraint({ 400, -1 }, { 500, -1 });
                    break;

                case 5:
                    // Aspect ratio
                    apply_constraint({ 0, 0 }, { FLT_MAX, FLT_MAX }, resize_contraints::aspect_ratio,
                                     static_cast<void*>(&aspect_ratio));
                    break;

                case 6:
                    // Always Square
                    apply_constraint({ 0, 0 }, { FLT_MAX, FLT_MAX }, resize_contraints::square);
                    break;

                case 7:
                    // Fixed Step
                    apply_constraint({ 0, 0 }, { FLT_MAX, FLT_MAX }, resize_contraints::step,
                                     static_cast<void*>(&fixed_step));
                    break;
            }

            if (!m_window_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ds::vector2<float>::zero());

            const ImGuiWindowFlags window_flags = m_auto_resize ? ImGuiWindowFlags_AlwaysAutoResize
                                                                : ImGuiWindowFlags_None;

            const bool window_open = ImGui::Begin("Example: Constrained Resize", m_open,
                                                  window_flags);
            if (!m_window_padding)
                ImGui::PopStyleVar();

            if (window_open)
            {
                if (ImGui::GetIO().KeyShift)
                {
                    // Display a viewport. could also use ImageButton() to display texture.
                    ds::vector2<f32> avail_size{ ImGui::GetContentRegionAvail() };
                    ds::vector2<f32> pos{ ImGui::GetCursorScreenPos() };

                    ImGui::ColorButton(
                        "viewport", { 0.5f, 0.2f, 0.5f, 1.0f },
                        ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, avail_size);

                    ImGui::SetCursorScreenPos({ pos.x + 10, pos.y + 10 });
                    ImGui::Text("%.2f x %.2f", avail_size.x, avail_size.y);
                }
                else
                {
                    ImGui::Text("(Hold SHIFT to display a dummy viewport)");

                    if (ImGui::Button("Set 200x200"))
                        ImGui::SetWindowSize({ 200, 200 });

                    ImGui::SameLine();
                    if (ImGui::Button("Set 500x500"))
                        ImGui::SetWindowSize({ 500, 500 });

                    ImGui::SameLine();
                    if (ImGui::Button("Set 800x200"))
                        ImGui::SetWindowSize({ 800, 200 });

                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 20);
                    ImGui::Combo("Constraint", &m_type, m_test_desc.data(),
                                 cast::to<i32>(m_test_desc.size()));

                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 20);
                    ImGui::DragInt("Lines", &m_display_lines, 0.2f, 1, 100);
                    ImGui::Checkbox("Auto-resize", &m_auto_resize);
                    ImGui::Checkbox("Window padding", &m_window_padding);

                    for (i32 i = 0; i < m_display_lines; i++)
                        ImGui::Text("%*sasdjdfnkjniuhfkljdsdihfskfjewiuwfkjsdnskjfnskn.", i * 4, "");
                }
            }

            ImGui::End();
        }
    };
}
