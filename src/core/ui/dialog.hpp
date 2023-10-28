#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "core/ds/color.hpp"
#include "core/ds/dimensions.hpp"
#include "core/ds/rect.hpp"
#include "core/input/input.hpp"
#include "core/input/mouse.hpp"
#include "core/ui/properties.hpp"
#include "core/ui/widget.hpp"
#include "thirdparty/raygui.hpp"

namespace rl::ui
{
    class Dialog : public Widget<Dialog>
    {
    public:
        constexpr void set_properties(ui::Properties&& props)
        {
            this->m_rect = ds::rect<int32_t>{
                std::move(props.position),
                std::move(props.size),
            };
            this->m_text = props.text;
        }

        constexpr bool check_collision(ds::point<int32_t>&& cursor_position) const
        {
            ds::rect<int32_t> status_bar{ m_rect };
            status_bar.height(RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT);
            if (status_bar.intersects(cursor_position))
                return true;

            return false;
        }

        auto handle_mouse_inputs(input::Input& input)
            -> std::expected<MouseEventCapture, InputCaptureError>
        {
            MouseEventCapture ret = MouseEventCapture::Unknown;

            using input::device::Mouse;
            const std::array button_states{ input.mouse_button_states() };
            const Mouse::ButtonState lmb_state{ button_states.at(Mouse::Button::Left) };

            switch (lmb_state)
            {
                case Mouse::ButtonState::Pressed:
                {
                    ds::point<int32_t>&& cursor_position{ input.mouse_cursor_position() };
                    if (this->check_collision(std::move(cursor_position)))
                    {
                        input.set_selection(true, m_id);
                        ret = MouseEventCapture::Grabbed;
                    }
                    break;
                }
                case Mouse::ButtonState::Held:
                {
                    // move selected control by mouse cursor delta since last update
                    auto&& [has_selection, selected_id] = input.get_selection();
                    if (has_selection && selected_id == m_id)
                    {
                        auto&& cursor_delta_movement{ input.mouse_cursor_delta() };
                        ret = this->move(std::move(cursor_delta_movement))
                                  ? MouseEventCapture::Dragging
                                  : MouseEventCapture::PartialDrag;
                    }
                    break;
                }
                case Mouse::ButtonState::Released:
                {
                    auto&& [has_selection, selected_id] = input.get_selection();
                    ret = has_selection && selected_id == m_id ? MouseEventCapture::Released
                                                               : MouseEventCapture::None;
                    // reset selection
                    input.set_selection(false);
                    break;
                }
                case Mouse::ButtonState::None:
                    break;
            }

            if (ret == MouseEventCapture::Unknown) [[unlikely]]
                return std::unexpected(InputCaptureError::Unknown);

            return ret;
        }

        /**
         * @brief Default control updates
         * */
        void process_updates()
        {
            return;
        }

        void draw()
        {
            raylib::GuiWindowBox(m_rect, m_text.c_str());
        }
    };
}
