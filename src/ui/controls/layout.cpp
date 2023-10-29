#include "ui/controls/layout.hpp"

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"

namespace rl::ui
{
    // bool layout::draw_impl()
    // {
    //     raylib::Rectangle panel_rect{
    //         static_cast<f32>(pos.x),
    //         static_cast<f32>(pos.y),
    //         static_cast<f32>(size.width),
    //         static_cast<f32>(size.height),
    //     };
    //     raylib::GuiPanel(panel_rect, text.c_str());
    //     return true;
    // }

    bool layout::inputs_impl(input::Input&)
    {
        return false;
    }

    bool layout::update_impl()
    {
        return false;
    }
}
