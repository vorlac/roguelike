#pragma once

#include <flecs.h>

#include "core/utils/io.hpp"
#include "ecs/components/ui_components.hpp"
#include "ecs/scenes/scene_types.hpp"

namespace rl::systems
{
    auto init_main_menu_systems(flecs::world& world)
    {
        // Will only run when the menu scene is currently active.
        world.system<const component::button>("Print Menu Button Text")
            .kind<scene::main_menu>()
            .each([](const component::button& b) {
                // Prints out the text of the menu button.
                log::info("Button label: '{}'", b.text);
            });
    }
}
