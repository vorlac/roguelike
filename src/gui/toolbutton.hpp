#pragma once

#include "gui/button.hpp"

namespace rl::gui {

    class ToolButton : public Button
    {
    public:
        ToolButton(Widget* parent, int icon, const std::string& caption = "")
            : Button(parent, caption, icon)
        {
            setFlags(Flags::RadioButton | Flags::ToggleButton);
            setFixedSize(Vector2i(25, 25));
        }
    };
}
