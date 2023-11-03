#pragma once

#include <imgui.h>
#include <fmt/format.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/io.hpp"

namespace rl::ui
{
    class fps_overlay
    {
    public:
        inline void update(const f32 frame_time, const i32 fps)
        {
            if (m_loc == Location::Custom)
                m_window_flags &= ~ImGuiWindowFlags_NoMove;
            else
            {
                const ds::point<f32> work_pos{ m_viewport->WorkPos };
                const ds::dimensions<f32> work_size{ m_viewport->WorkSize };

                auto window_pos = ds::point<f32>{
                    (m_loc & 1) != 0 ? work_pos.x + work_size.width - offset : work_pos.x + offset,
                    (m_loc & 2) != 0 ? work_pos.y + work_size.height - offset : work_pos.y + offset,
                };

                auto window_pos_pivot = ds::point<f32>{
                    m_loc & 1 ? 1.0f : 0.0f,
                    m_loc & 2 ? 1.0f : 0.0f,
                };

                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                m_window_flags |= ImGuiWindowFlags_NoMove;
            }

            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::Begin("Debug Overlay", nullptr, m_window_flags))
            {
                ImGui::Text("Diagnostics");
                ImGui::Separator();

                std::string&& line1{ fmt::format("Cursor Position: {}", m_io.MousePos).data() };
                std::string&& line2{ fmt::format("Frametime: {:2.6f} ms\n", frame_time).data() };
                std::string&& line3{ fmt::format("FPS: {}\n", fps).data() };
                ImGui::Text(line1.data());
                ImGui::Text(line2.data());
                ImGui::Text(line3.data());

                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem("Custom", nullptr, m_loc == Location::Custom))
                        m_loc = Location::Custom;
                    if (ImGui::MenuItem("Move to top left", nullptr, m_loc == Location::TopLeft))
                        m_loc = Location::TopLeft;
                    if (ImGui::MenuItem("Move to top right", nullptr, m_loc == Location::TopRight))
                        m_loc = Location::TopRight;
                    if (ImGui::MenuItem("Move to bot left", nullptr, m_loc == Location::BotLeft))
                        m_loc = Location::BotLeft;
                    if (ImGui::MenuItem("Move to bot right", nullptr, m_loc == Location::BotRight))
                        m_loc = Location::BotRight;

                    ImGui::EndPopup();
                }
            }

            ImGui::End();
        }

    private:
        enum Location : i32 {
            Custom   = -1,
            TopLeft  = 0,
            TopRight = 1,
            BotLeft  = 2,
            BotRight = 3,
        };

        Location m_loc{ Location::TopLeft };
        ImGuiIO& m_io{ ImGui::GetIO() };
        ImGuiWindowFlags m_window_flags{
            ImGuiWindowFlags_NoDecoration |        // No TitleBar, Resize, Scrollbar, Collapse
            ImGuiWindowFlags_AlwaysAutoResize |    // Resize window to fit it's contents
            ImGuiWindowFlags_NoSavedSettings |     // Don't use imgui.ini
            ImGuiWindowFlags_NoFocusOnAppearing |  // Disables stealing focus after showing
            ImGuiWindowFlags_NoNav                 // Don't focus for kb/gamepad navigation
        };

        static inline bool m_show{ true };
        static constexpr inline f32 offset{ 10.0f };
        const ImGuiViewport* m_viewport{ ImGui::GetMainViewport() };
    };
}
