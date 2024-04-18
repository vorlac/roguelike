#pragma once

#include "ui/widget.hpp"

namespace rl::ui {

    class Panel final : public Widget
    {
    public:
        explicit Panel(Widget* parent);
    };
}
