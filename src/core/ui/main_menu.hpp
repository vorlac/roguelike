#pragma once

#include <array>
#include <string>

#include <imgui.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ui/imgui_helpers.hpp"

namespace rl::ui
{
    class main_menu
    {
    private:
        static inline bool m_is_open{ false };
        static inline constexpr bool m_use_work_area{ false };
        ImGuiIO& m_io{ ImGui::GetIO() };
        const ImGuiViewport* m_viewport{ ImGui::GetMainViewport() };
        static inline ImGuiWindowFlags m_flags = ImGuiWindowFlags_NoDecoration |
                                                 ImGuiWindowFlags_NoTitleBar |
                                                 ImGuiWindowFlags_NoMove |
                                                 ImGuiWindowFlags_NoSavedSettings |
                                                 ImGuiWindowFlags_AlwaysAutoResize |
                                                 ImGuiWindowFlags_NoDecoration;

        static constexpr std::array menu_labels{
            "Start Game",
            "Options",
            "Credits",
            "Exit To Desktop",
        };

    public:
        inline void update()
        {
            this->apply_style();
            // We demonstrate using the full viewport area or the work area (without menu-bars,
            // task-bars etc.) Based on your use case you may want one or the other.
            ImGui::SetNextWindowPos(m_use_work_area ? m_viewport->WorkPos : m_viewport->Pos);
            ImGui::SetNextWindowSize(m_use_work_area ? m_viewport->WorkSize : m_viewport->Size);

            if (!ImGui::Begin("Main Menu", &m_is_open, m_flags))
                ImGui::End();
            else
            {
                // ImGui::Checkbox("Use work area instead of main area", &m_use_work_area);

                ds::point<i32>&& center{ m_viewport->GetCenter() };
                ds::dimensions<i32>&& size{ m_viewport->WorkSize };
                auto scaled_rect =
                    ds::rect<i32>({ center - (size / 2) }, size).scaled({ 0.75f, 0.5f });

                static i32 table_flags = ImGuiTableColumnFlags_WidthStretch |  //
                                         ImGuiTableColumnFlags_NoHide |        //
                                         ImGuiTableColumnFlags_NoClip |        //
                                         ImGuiTableColumnFlags_NoSort |        //
                                         ImGuiTableColumnFlags_NoHeaderLabel;  //
                i32 prev_font_size{ m_io.Fonts->ConfigData.Size };
                m_io.Fonts->ConfigData.Size = 64;
                m_io.FontAllowUserScaling   = true;
                m_io.FontGlobalScale        = 10;

                ImGui::BeginTable("Main Menu Table", 3, table_flags);
                for (auto&& menu_item : menu_labels)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Button(
                        menu_item,
                        { m_viewport->WorkSize.x,
                          m_viewport->WorkSize.y / cast::to<f32>((menu_labels.size() + 1)) });
                }

                ImGui::EndTable();
                m_io.Fonts->ConfigData.Size = prev_font_size;

                m_io.FontGlobalScale = 1;
            }
            ImGui::End();
        }

    private:
        inline void apply_style()
        {
            // ui properties
            ImGuiStyle& style{ ImGui::GetStyle() };

            // main
            style.WindowPadding     = { 10.0f, 10.0f };
            style.ItemSpacing       = { 10.0f, 10.0f };
            style.ItemInnerSpacing  = { 10.0f, 10.0f };
            style.FramePadding      = { 5.0f, 5.0f };
            style.CellPadding       = { 5.0f, 5.0f };
            style.TouchExtraPadding = { 0.0f, 0.0f };
            style.IndentSpacing     = { 20.0f };
            style.ScrollbarSize     = { 18.0f };
            style.GrabMinSize       = { 10.0f };

            // borders
            style.WindowBorderSize = { 0.0f };
            style.ChildBorderSize  = { 0.0f };
            style.PopupBorderSize  = { 0.0f };
            style.FrameBorderSize  = { 0.0f };
            style.TabBorderSize    = { 0.0f };

            // rounding
            style.WindowRounding    = { 0.0f };  //{ 6.0f };
            style.ChildRounding     = { 0.0f };
            style.FrameRounding     = { 0.0f };
            style.PopupRounding     = { 5.0f };
            style.ScrollbarRounding = { 10.0f };
            style.GrabRounding      = { 0.0f };
            style.TabRounding       = { 1.0f };

            // widgets
            style.WindowTitleAlign         = { 0.5f, 0.5f };
            style.WindowMenuButtonPosition = { ImGuiDir_::ImGuiDir_None };
            style.ColorButtonPosition      = { ImGuiDir_::ImGuiDir_Right };
            style.SelectableTextAlign      = { 0.5, 0.5 };
            style.ButtonTextAlign          = { 0.5f, 0.5f };
            // style.SelectableTextAlign      = { 0.0f, 0.0f };
            style.SeparatorTextBorderSize = { 3.0f };
            style.SeparatorTextAlign      = { 0.25f, 0.50f };
            style.SeparatorTextPadding    = { 0.0f, 10.0f };
            style.LogSliderDeadzone       = { 5.0f };

            // tooltips
            style.HoverFlagsForTooltipMouse = ImGuiHoveredFlags_::ImGuiHoveredFlags_DelayShort |
                                              ImGuiHoveredFlags_::ImGuiHoveredFlags_Stationary;
            style.HoverFlagsForTooltipNav = ImGuiHoveredFlags_::ImGuiHoveredFlags_DelayNormal |
                                            ImGuiHoveredFlags_::ImGuiHoveredFlags_NoSharedDelay;
            style.HoverDelayShort = true;

            // misc
            style.DisplaySafeAreaPadding = { 3, 3 };

            style.Alpha = { 1.0f };
            ImGui::StyleColorsDark(&style);
        }
    };
}
