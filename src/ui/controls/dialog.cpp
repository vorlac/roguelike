#include "ui/controls/dialog.hpp"

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"

namespace rl::ui
{

    constexpr bool dialog::reposition(ds::vector2<i32>&& movement_offset)
    {
        this->pos += movement_offset;
        return true;
    }

    constexpr bool dialog::inputs_impl(input::Input& input)
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

        if (ret == MouseEventCapture::Unknown)
            return false;

        return ret != MouseEventCapture::None;
    }

}
