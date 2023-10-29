#pragma once

#include "ui/control.hpp"

namespace rl::input
{
    class Input;
}

namespace rl::ds
{
    template <typename T>
        requires Numeric<T>
    struct vector2;
}

namespace rl::ui
{
    class dialog : public control
    {
    public:
        using control::control;

        bool inputs_impl(input::Input& input);
        bool reposition(ds::vector2<i32>&& movement_offset);

        // constexpr bool check_collision(ds::point<i32>&& cursor_position);

        constexpr inline bool check_collision(ds::point<i32>&& cursor_position)
        {
            // check to see if the status bar rectangle intersects with the cursor
            // since that should be the only interactive portion of a ui::dialog
            return ds::rect<i32>{
                ds::point<i32>{ this->pos }, ds::dimensions<i32>{ this->size.width, 24U }
            }.intersects(cursor_position);
        }

        bool draw_impl()
        {
            raylib::Rectangle window_rect{
                static_cast<f32>(pos.x),
                static_cast<f32>(pos.y),
                static_cast<f32>(size.width),
                static_cast<f32>(size.height),
            };
            raylib::GuiWindowBox(window_rect, text.c_str());
            return true;
        }
    };
}
