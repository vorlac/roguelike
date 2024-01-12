#pragma once

#include <string>

#include "core/ui/button.hpp"

namespace rl::ui {

    // Simple radio+toggle button with an icon.
    class ToolButton : public ui::Button
    {
    public:
        ToolButton(ui::Widget* parent, ui::Icon::ID icon, const std::string& caption = "")
            : ui::Button(parent, caption, icon)
        {
            this->set_flags(Button::Flags::ToolButton);
            this->set_fixed_size(ds::dims<i32>{ 25, 25 });
        }
    };
}
