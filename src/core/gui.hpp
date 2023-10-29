#pragma once

#include "core/input/input.hpp"

#include "thirdparty/raygui.hpp"

namespace rl::input
{
    class Input;
}

namespace rl::ui
{
    class dialog;

    class GUI
    {
    public:
        constexpr GUI();

        constexpr ~GUI() = default;

        inline bool update(this auto&& self, input::Input& input);
        inline bool render(this auto&& self);

    protected:
        std::shared_ptr<ui::dialog> m_test_dialog{};
        input::Input m_input{};
    };
}
