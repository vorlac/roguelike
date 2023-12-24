#pragma once

#include "gui/button.hpp"

namespace rl::gui {

    /**
     * \class ToolButton toolbutton.h nanogui/toolbutton.h
     *
     * \brief Simple radio+toggle button with an icon.
     */
    class ToolButton : public Button
    {
    public:
        ToolButton(Widget* parent, int icon, const std::string& caption = "")
            : Button(parent, caption, icon)
        {
            set_flags(Flags::RadioButton | Flags::ToggleButton);
            set_fixed_size(Vector2i(25, 25));
        }
    };

}
