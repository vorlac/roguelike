#pragma once

#include <utility>
#include <vector>

#include "core/input/input.hpp"
#include "ui/controls/dialog.hpp"

namespace rl::ui
{
    class GUI
    {
    public:
        inline bool update(input::Input& input)
        {
            return m_test_dialog.update(input);
        }

        inline bool render()
        {
            return m_test_dialog.draw();
        }

    private:
        // std::shared_ptr<ui::control> m_test_dialog{};  // ui::dialog::create() };
        ui::dialog m_test_dialog{ {
            .text     = std::string{ "asdsdasa" },
            .size     = ds::dimensions<i32>{ 800, 600 },
            .position = ds::point<i32>{ 100, 100 },
        } };

        input::Input m_input{};
    };
}
