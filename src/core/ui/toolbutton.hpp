#pragma once

#include <string>

#include "core/ui/button.hpp"

namespace rl::ui {
    // Simple radio+toggle button with an icon.
    class ToolButton : public Button
    {
    public:
        ToolButton(Widget* parent, const Icon::ID icon, const std::string& caption = "")
            : Button(parent, caption, icon)
        {
            this->set_property(Button::Property::Toolbar);
            this->set_fixed_size(ds::dims<f32>{ 25.0f, 25.0f });
        }
    };
}
