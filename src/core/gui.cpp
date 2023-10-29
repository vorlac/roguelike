#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/gui.hpp"
#include "core/input/input.hpp"
#include "core/numerics.hpp"
#include "ui/control.hpp"
#include "ui/controls/dialog.hpp"
#include "ui/properties.hpp"

namespace rl::ui
{
    GUI::GUI()
    {
        m_test_dialog = new dialog(
            { "asdsdasa", ds::dimensions<i32>{ 800, 600 }, ds::point<i32>{ 100, 100 } });
    }

}
