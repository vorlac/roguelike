#include "ui/control.hpp"

#include "ui/properties.hpp"

namespace rl::ui
{
    // bool control::inputs_impl(input::Input& inputs)
    // {
    //     bool inputs_captured{ false };
    //     for (auto&& child : children)
    //     {
    //         if (!child.visible || !child.enabled)
    //             continue;

    //         inputs_captured |= child.update_gui(inputs);
    //         if (inputs_captured)
    //             break;
    //     }

    //     if (!inputs_captured)  // && !processed_inputs)
    //         inputs_captured |= this->inputs_impl(inputs);

    //     return inputs_captured;
    // }

    // constexpr inline bool control::update_gui(this auto&& self, input::Input& inputs)
    // {
    //     return self.inputs_impl(inputs);
    // }

    // constexpr inline bool control::draw(this auto&& self)
    // {
    //     return self.draw_impl();
    // }
}
