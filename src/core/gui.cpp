#include <string>

#include "core/ds/dimensions.hpp"
#include "core/gui.hpp"
#include "input/input.hpp"
#include "ui/controls/dialog.hpp"

#include "thirdparty/raygui.hpp"

namespace rl::ui
{

    // GUI::GUI()
    //     : m_test_dialog{ new dialog(
    //           { "asdsdasa", ds::dimensions<i32>{ 800, 600 }, ds::point<i32>{ 100, 100 } }) }
    // {
    // }

    inline bool GUI::update(this auto&& self, input::Input& input)
    {
        if (m_test_dialog)
            m_test_dialog->update_gui(input);
        return false;
    }

    inline bool GUI::render(this auto&& self)
    {
        return self->draw();
    }
}
