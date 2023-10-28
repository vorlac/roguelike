#pragma once

#include <expected>
#include <string>
#include <vector>

#include "core/ds/color.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/input/input.hpp"
#include "core/input/mouse.hpp"
#include "core/numerics.hpp"
#include "thirdparty/raygui.hpp"
#include "ui/controls/control.hpp"
#include "ui/properties.hpp"

namespace rl::ui
{
    class dialog : public ui::control
    {
    public:
        using ui::control::control;

        constexpr bool check_collision(ds::point<i32>&& cursor_position) const
        {
            // check to see if the status bar rectangle intersects with the cursor
            // since that should be the only interactive portion of a ui::dialog
            return ds::rect<i32>{
                ds::point<i32>{ this->pos },
                ds::dimensions<i32>{ this->size.width, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT }
            }.intersects(cursor_position);
        }

        inline bool reposition(ds::vector2<i32>&& movement_offset)
        {
            this->pos += movement_offset;
            return true;
        }

        constexpr inline bool inputs_impl(input::Input& input)
        {
            MouseEventCapture ret = MouseEventCapture::Unknown;

            using input::device::Mouse;
            const std::array button_states{ input.mouse_button_states() };
            const Mouse::ButtonState lmb_state{ button_states.at(Mouse::Button::Left) };

            switch (lmb_state)
            {
                case Mouse::ButtonState::Pressed:
                {
                    ds::point<i32>&& cursor_position{ input.mouse_cursor_position() };
                    if (this->check_collision(std::move(cursor_position)))
                    {
                        input.set_selection(true, this->id);
                        ret = MouseEventCapture::Grabbed;
                    }
                    break;
                }
                case Mouse::ButtonState::Held:
                {
                    // move selected control by mouse cursor delta since last update
                    auto&& [has_selection, selected_id] = input.get_selection();
                    if (has_selection && selected_id == this->id)
                    {
                        auto&& cursor_delta_movement{ input.mouse_cursor_delta() };
                        ret = this->reposition(std::move(cursor_delta_movement))
                                  ? MouseEventCapture::Dragging
                                  : MouseEventCapture::PartialDrag;
                    }
                    break;
                }
                case Mouse::ButtonState::Released:
                {
                    auto&& [has_selection, selected_id] = input.get_selection();
                    ret = has_selection && selected_id == this->id ? MouseEventCapture::Released
                                                                   : MouseEventCapture::None;
                    input.set_selection(false);
                    break;
                }
                case Mouse::ButtonState::None:
                    break;
            }

            if (ret == MouseEventCapture::Unknown) [[unlikely]]
                return false;

            return ret != MouseEventCapture::None;
        }

        inline bool draw_impl()
        {
            ds::rect<f32> window_rect{
                ds::point<f32>{
                    static_cast<f32>(this->pos.x),
                    static_cast<f32>(this->pos.y),
                },
                ds::dimensions<f32>{
                    static_cast<f32>(this->size.width),
                    static_cast<f32>(this->size.height),
                },
            };
            raylib::GuiWindowBox(window_rect, text.c_str());
            return true;
        }

    private:
        constexpr inline std::string name()
        {
            return "ui::dialog";
        }
    };
}
