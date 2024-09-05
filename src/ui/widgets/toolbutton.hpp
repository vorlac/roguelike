#pragma once

#include <string>

#include "ui/widgets/button.hpp"

namespace rl::ui {

    // Simple radio+toggle button with an icon.
    class ToolButton final : public Button {
    public:
        ToolButton(Widget* parent, const Icon::ID icon, std::string&& caption = "")
            : Button(parent, std::forward<std::string>(caption), icon) {
            this->set_property(Button::Property::Toolbar);
            this->set_fixed_size(ds::dims<f32>{ 25.0f, 25.0f });
        }
    };
}
