#pragma once

#include <string>

#include "core/ui/button.hpp"

namespace rl::ui {

    /// @brief
    ///     Simple radio+toggle button with an icon.
    class ToolButton : public ui::Button
    {
    public:
        ToolButton(ui::widget* parent, ui::Icon icon, const std::string& caption = "")
            : ui::Button(parent, caption, icon)
        {
            this->set_flags(Button::Flags(Button::Flags::RadioButton | Button::Flags::ToggleButton));
            this->set_fixed_size(ds::dims<i32>{ 25, 25 });
        }
    };
}
