#pragma once

#include <utility>
#include <vector>
#include <imgui.h>

#include "core/input.hpp"
#include "thirdparty/rlimgui.hpp"

namespace rl
{
    class GUI
    {
    public:
        inline void setup() const
        {
            rlimgui::Setup(true);
        }

        inline void teardown() const
        {
            rlimgui::Shutdown();
        }

        inline bool update(Input&)
        {
            rlimgui::Begin();
            this->draw();
            rlimgui::End();
            return true;
        }

    private:
        inline bool draw()
        {
            bool open = true;
            ImGui::ShowDemoWindow(&open);
            return true;
        }
    };
}
