#pragma once

#include <utility>
#include <vector>
#include <imgui.h>

#include "core/input/input.hpp"

namespace rl::ui
{
    class GUI
    {
    public:
        inline bool update(input::Input&)
        {
            return false;
        }

        inline bool render()
        {
            bool open = true;
            ImGui::ShowDemoWindow(&open);
            return true;
        }

    private:
        input::Input m_input{};
    };
}
