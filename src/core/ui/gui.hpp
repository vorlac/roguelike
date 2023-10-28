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
        template <typename TControl>
        void add_control(TControl&& control)
        {
            m_controls = std::move(control);
        }

        void draw_gui()
        {
            m_controls->render();
        }

        void update_gui(rl::input::Input& input)
        {
            using input::device::Mouse;

            m_controls->handle_inputs(input);
        }

    private:
        template <typename TControl>
        static constexpr inline auto update_gui_controls(TControl* widget, input::Input& input)
        {
            return widget->handle_inputs(input);
        }

    private:
        ui::Dialog* m_controls{};

        input::Input m_input{};
    };
}
