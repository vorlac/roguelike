#include "ui/widgets/panel.hpp"

namespace rl::ui {
    Panel::Panel(Widget* parent)
        : Widget{ parent }
    {
        if (parent != nullptr)
            parent->add_child(this);
    }
}
