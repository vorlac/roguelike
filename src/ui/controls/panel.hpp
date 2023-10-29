#pragma once

#include "ui/control.hpp"

#include "thirdparty/raygui.hpp"

namespace rl::input
{
    class Input;
}

namespace rl::ui
{
    class panel : public control
    {
    public:
        using control::control;

        // bool inputs_impl(input::Input& input);

        // bool update_impl();

        inline bool draw_impl()
        {
            raylib::Rectangle panel_rect{
                static_cast<f32>(pos.x),
                static_cast<f32>(pos.y),
                static_cast<f32>(size.width),
                static_cast<f32>(size.height),
            };
            raylib::GuiPanel(panel_rect, text.c_str());
            return true;
        }
    };
}
