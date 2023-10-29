#include "ui/controls/button.hpp"

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"

namespace rl::ui
{
    // bool button::draw_impl()
    // {
    //     raylib::Rectangle button_rect{
    //         static_cast<f32>(pos.x),
    //         static_cast<f32>(pos.y),
    //         static_cast<f32>(size.width),
    //         static_cast<f32>(size.height),
    //     };
    //     raylib::GuiLabelButton(button_rect, text.c_str());
    //     return true;
    // }

    bool button::inputs_impl(input::Input&)
    {
        return false;
    }

    // bool button::update_impl()
    //{
    //     return false;
    // }
}
