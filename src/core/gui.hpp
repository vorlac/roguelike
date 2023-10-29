#pragma once

#include "core/input/input.hpp"
// #include "thirdparty/raygui.hpp"
#include "ui/controls/dialog.hpp"

namespace rl::input
{
    class Input;
}

namespace rl::ui
{
    class GUI
    {
    public:
        GUI();
        ~GUI() = default;

        bool update(input::Input& input)
        {
            return m_test_dialog->update(input);
        }

        bool render()
        {
            return m_test_dialog->draw();
        }

    private:
        ui::dialog* m_test_dialog;
        input::Input m_input{};
    };
}
