#pragma once

#include "core/ui/widget.hpp"

namespace rl::ui {

    class Panel final : public Widget
    {
    public:
        explicit Panel(Widget* parent);
    };
}
