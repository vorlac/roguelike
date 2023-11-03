#pragma once

#include <string>

#include <imgui.h>

#include "core/numeric_types.hpp"

namespace rl::ui
{
    struct style
    {
        struct font
        {
            font(std::string&& ttf_path, f32 size_pt)
            {
                size = size_pt;
                path = ttf_path;
                // prevents font data from being freed
                config.FontDataOwnedByAtlas = false;
            }

            std::string path{};
            f32 size{ 18.0f };
            ImFontConfig config{};
        };

        static inline auto load()
        {
            {
                // fonts
                ImGuiIO& io{ ImGui::GetIO() };
                io.Fonts->AddFontFromFileTTF(m_firacode.path.data(), m_firacode.size,
                                             &m_firacode.config);
                m_firacode.size = 64;
                io.Fonts->AddFontFromFileTTF(m_firacode.path.data(), m_firacode.size,
                                             &m_firacode.config);
            }

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
                style.WindowRounding    = { 6.0f };
                style.ChildRounding     = { 6.0f };
                style.FrameRounding     = { 2.0f };
                style.PopupRounding     = { 5.0f };
                style.ScrollbarRounding = { 10.0f };
                style.GrabRounding      = { 2.0f };
                style.TabRounding       = { 1.0f };

                // widgets
                style.WindowTitleAlign         = { 0.5f, 0.5f };
                style.WindowMenuButtonPosition = { ImGuiDir_::ImGuiDir_None };
                style.ColorButtonPosition      = { ImGuiDir_::ImGuiDir_Right };
                style.ButtonTextAlign          = { 0.5f, 0.5f };
                style.SelectableTextAlign      = { 0.0f, 0.0f };
                style.SeparatorTextBorderSize  = { 3.0f };
                style.SeparatorTextAlign       = { 0.25f, 0.50f };
                style.SeparatorTextPadding     = { 0.0f, 10.0f };
                style.LogSliderDeadzone        = { 5.0f };

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
        }

        static inline style::font m_firacode{ "./data/fonts/fira-code-retina.ttf", 18.0f };
    };
}
