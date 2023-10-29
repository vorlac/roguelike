#pragma once

#include "thirdparty/raygui.hpp"
#include "ui/control.hpp"

namespace rl::input
{
    class Input;
}

namespace rl::ui
{
    class label : public control
    {
    public:
        using control::control;

        bool inputs_impl(input::Input& input);

        // bool update_impl();

        bool draw_impl()
        {
            raylib::Rectangle label_rect{
                static_cast<f32>(pos.x),
                static_cast<f32>(pos.y),
                static_cast<f32>(size.width),
                static_cast<f32>(size.height),
            };
            raylib::GuiLabel(label_rect, text.c_str());
            return true;
        }
    };
}
