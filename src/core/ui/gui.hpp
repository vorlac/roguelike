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

        void update_gui(auto&& button_states, auto&& cursor_pos)
        {
            using input::device::Mouse;
            Mouse::ButtonID lmb = static_cast<uint32_t>(Mouse::Button::Left);
            for (auto&& control : m_controls)
            {
                switch (control->type())
                {
                    case Control::Type::Dialog:
                    {
                        if (button_states[lmb] == Mouse::ButtonState::Pressed)
                        {
                            Dialog* dialog = static_cast<Dialog*>(control.get());
                            dialog->check_collision(cursor_pos);
                        }
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

    private:
        std::vector<std::shared_ptr<Control>> m_controls{};
        input::Input m_input{};
    };
}
