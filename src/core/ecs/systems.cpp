#pragma once

#include "core/ecs/systems.hpp"

#include <iostream>

#include "core/ecs/components.hpp"
#include "core/ecs/scenes.hpp"

namespace rl::systems
{
    void init_level_systems(flecs::world& world)
    {
        // Will run every time regardless of the current scene we're in.
        world.system<const components::Position>("Print Position")
            .each([](flecs::entity e, const components::Position& p) {
                // Prints out the position of the entity.
                std::cout << e.name() << ": {" << p.x << ", " << p.y << "}\n";
            });

        // Will only run when the game scene is currently active.
        world.system<components::Health>("Characters Lose Health")
            .kind<scenes::Level>()
            .each([](components::Health& h) {
                // Prints out the character's health and then decrements it by one.
                std::cout << h.amount << " health remaining\n";
                h.amount--;
            });

        // Will only run when the menu scene is currently active.
        world.system<const components::Button>("Print Menu Button Text")
            .kind<scenes::MainMenu>()
            .each([](const components::Button& b) {
                // Prints out the text of the menu button.
                std::cout << "Button says \"" << b.text << "\"\n";
            });
    }
}
