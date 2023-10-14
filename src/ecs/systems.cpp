#pragma once

#include "ecs/systems.hpp"

#include <iostream>
#include <flecs.h>
#include <raylib.h>

#include "ds/vector2d.hpp"
#include "ecs/components.hpp"
#include "ecs/scenes.hpp"
#include "utils/color.hpp"

namespace rl::systems
{
    void init_level_systems(flecs::world& world)
    {
        // Will run every time regardless of the current scene we're in.
        // world.system<Vector2, Vector2>("Print Position")
        //    .each([](flecs::entity, Vector2& p, Vector2& v) {
        //        std::cout << "(" << p.x << ", " << v.y << ")" << std::endl;
        //    })
        //    .add_flags(flecs::IsEntity | flecs::Pipeline);

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
