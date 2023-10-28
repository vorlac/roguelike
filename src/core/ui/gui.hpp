#pragma once

#include <vector>

#include "core/input/input.hpp"
#include "core/ui/control.hpp"
#include "core/ui/dialog.hpp"

namespace rl::ui
{
    class GUI
    {
    public:
        /**
         * @brief Adds top level controls/widgets to be managed
         * @param control Accepts a TControl*, TControl&, or shared_ptr<Control>
         * of any GUI element derived from the ui::Control class
         * */
        template <typename TControl = Dialog>
        void add_control(TControl control)
        {
            m_controls.emplace_back(static_cast<Control*>(control));
        }

        void draw_gui()
        {
            for (auto&& control : m_controls)
            {
                switch (control->type())
                {
                    case Control::Type::Dialog:
                    {
                        Dialog* dialog = static_cast<Dialog*>(control.get());
                        dialog->draw();
                        break;
                    }
                    case Control::Type::Control:
                    case Control::Type::Button:
                    case Control::Type::ToggleButton:
                    case Control::Type::Checkbox:
                        break;
                }
            }
        }

        void update_gui(rl::input::Input& input)
        {
            using input::device::Mouse;

            const std::array button_states{ input.mouse_button_states() };
            const ds::point<int32_t> cursor_position{ input.mouse_cursor_position() };
            const ds::vector2<int32_t> cursor_delta_pos{ input.mouse_cursor_delta() };

            Mouse::ButtonID lmb = static_cast<uint32_t>(Mouse::Button::Left);
            for (auto&& control : m_controls)
            {
                // for each top level ui control
                switch (control->type())
                {
                    case Control::Type::Control:
                    case Control::Type::Button:
                    case Control::Type::ToggleButton:
                    case Control::Type::Checkbox:
                        break;

                    case Control::Type::Dialog:
                    {
                        Dialog* dialog = static_cast<Dialog*>(control.get());
                        const Mouse::ButtonState lmb_state{ button_states[lmb] };
                        switch (lmb_state)
                        {
                            case Mouse::ButtonState::Pressed:
                            {
                                uint64_t addr{ NULL };
                                if (dialog->check_collision(cursor_position))
                                    addr = reinterpret_cast<uint64_t>(dialog);
                                input.set_selection(addr != NULL, addr);
                                break;
                            }
                            case Mouse::ButtonState::Held:
                            {
                                auto&& [has_selection, selected_id] = input.get_selection();
                                if (has_selection)
                                {
                                    // move selected control by mouse
                                    // cursor delta since last update
                                    uint64_t addr{ reinterpret_cast<uint64_t>(dialog) };
                                    if (addr == selected_id)
                                        dialog->move(cursor_delta_pos);
                                }
                                break;
                            }
                            case Mouse::ButtonState::Released:
                                // deselect
                                input.set_selection(false);
                                break;
                        }

                        break;
                    }
                }
            }
        }

    private:
        std::vector<std::shared_ptr<Control>> m_controls{};
        input::Input m_input{};
    };
}
