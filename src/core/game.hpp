#pragma once

#include <memory>
#include <utility>
#include <flecs.h>

#include "core/application.hpp"
#include "core/ds/dimensions.hpp"
#include "core/gui.hpp"
#include "core/input/input.hpp"

#include "thirdparty/raygui.hpp"

namespace rl
{
    class Game : public Application
    {
    public:
        Game()
            : m_gui{ std::make_unique<ui::GUI>() }
        {
        }

        constexpr ~Game() = default;

        bool setup();
        bool run();
        bool teardown();

        bool should_quit() const;
        void quit() const;

    protected:
        flecs::world m_world{};
        std::unique_ptr<ui::GUI> m_gui{ std::make_unique<ui::GUI>() };
    };
}
