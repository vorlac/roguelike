#pragma once

#include <string>
#include <string_view>

#include <imgui.h>

#include "core/numeric_types.hpp"
#include "core/ui/imgui_helpers.hpp"
#include "core/utils/assert.hpp"

namespace rl::ui
{
    class file_menu
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
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    this->show_file_menu();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    bool undo_selected{ ImGui::MenuItem("Undo", "CTRL+Z") };
                    bool redo_selected{ ImGui::MenuItem("Redo", "CTRL+Y", false, false) };

                    ImGui::Separator();

                    bool cut_selected{ ImGui::MenuItem("Cut", "CTRL+X") };
                    bool copy_selected{ ImGui::MenuItem("Copy", "CTRL+C") };
                    bool paste_selected{ ImGui::MenuItem("Paste", "CTRL+V") };

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

    private:
        ui::event_callback m_callback = [](std::string file, i32 line, std::string section,
                                           void* user_data) -> void {
            std::string fileno{ fileno };
            i32 line_num{ line };
            std::string ni_section{ section };
            void* ud{ user_data };
            return;
        };

        // Note that shortcuts are currently provided for display only
        // (future version will add explicit flags to BeginMenu() to request processing shortcuts)
        inline void show_file_menu()
        {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            bool new_selected{ ImGui::MenuItem("New") };
            bool open_selected{ ImGui::MenuItem("Open", "Ctrl+O") };

            if (ImGui::BeginMenu("Open Recent"))
            {
                ImGui::MenuItem("fish_hat.c");
                ImGui::MenuItem("fish_hat.inl");
                ImGui::MenuItem("fish_hat.h");
                if (ImGui::BeginMenu("More.."))
                {
                    ImGui::MenuItem("Hello");
                    ImGui::MenuItem("Sailor");
                    if (ImGui::BeginMenu("Recurse.."))
                    {
                        this->show_file_menu();
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            bool save_selected{ ImGui::MenuItem("Save", "Ctrl+S") };
            bool saveas_selected{ ImGui::MenuItem("Save As..") };

            ImGui::Separator();

            if (ImGui::BeginMenu("Options"))
            {
                static bool enabled = true;
                ImGui::MenuItem("Enabled", "", &enabled);
                ImGui::BeginChild("child", { 0, 60 }, true);

                for (i32 i = 0; i < 10; ++i)
                    ImGui::Text("Scrolling Text %d", i);

                ImGui::EndChild();
                static f32 f{ 0.5f };
                static i32 n{ 0 };
                ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
                ImGui::InputFloat("Input", &f, 0.1f);
                ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Colors"))
            {
                f32 sz{ ImGui::GetTextLineHeight() };
                for (ImGuiCol i = 0; i < ImGuiCol_COUNT; i++)
                {
                    std::string_view name{ ImGui::GetStyleColorName(i) };
                    u32 color_code{ ImGui::GetColorU32(i) };
                    ds::point<f32> p{ ImGui::GetCursorScreenPos() };

                    ImGui::GetWindowDrawList()->AddRectFilled(p, { p.x + sz, p.y + sz }, color_code);
                    ImGui::Dummy({ sz, sz });
                    ImGui::SameLine();
                    ImGui::MenuItem(name.data());
                }

                ImGui::EndMenu();
            }

            // append options menu item
            if (ImGui::BeginMenu("Options"))
            {
                void* cb_user_data{ nullptr };
                if (ui::file_menu::m_callback != nullptr)
                    ui::file_menu::m_callback(
                        __FILE__, __LINE__, "Examples/Menu/Append to an existing menu", cb_user_data);

                // IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
                static bool b = true;
                ImGui::Checkbox("SomeOption", &b);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Disabled", false))
                assertion(false, "disabled menu item selected");

            ImGui::MenuItem("Checked", NULL, true);
            ImGui::Separator();
            ImGui::MenuItem("Quit", "Alt+F4");
        }
    };
}
