#include "ui/controls/label.hpp"

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/numerics.hpp"

namespace rl::ui
{
    // bool label::draw_impl()
    // {
    //     raylib::Rectangle label_rect{
    //         static_cast<f32>(pos.x),
    //         static_cast<f32>(pos.y),
    //         static_cast<f32>(size.width),
    //         static_cast<f32>(size.height),
    //     };
    //     raylib::GuiLabel(label_rect, text.c_str());
    //     return true;
    // }

    bool label::inputs_impl(input::Input&)
    {
        return false;
    }

    // bool label::update_impl()
    // {
    //     return false;
    // }
}
