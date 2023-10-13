#pragma once

#include "core/ecs/observers.hpp"

#include <iostream>
#include <flecs.h>

#include "core/ecs/components.hpp"
#include "core/ecs/scenes.hpp"

namespace rl::observers
{
    // Removes all entities who are children of the current scene root.
    void reset_scene(flecs::world& world)
    {
        world.defer_begin();
        world.delete_with(flecs::ChildOf, world.entity<scenes::SceneRoot>());
        world.defer_end();
    }

    void add_level_observers(flecs::world& world)
    {
        // Observer to call scene change logic for scene::MainMenu when added to the ActiveScene.
        world.observer<scenes::ActiveScene>("Scene Change to Menu")
            .event(flecs::OnAdd)
            .second<scenes::MainMenu>()
            .each([](flecs::iter& it, size_t, scenes::ActiveScene) {
                std::cout << "\n>> ActiveScene has changed to `scene::MainMenu`\n\n";

                flecs::world ecs = it.world();
                flecs::entity scene = ecs.component<scenes::SceneRoot>();

                // Removes all entities who are children of the current scene root.
                reset_scene(ecs);

                // Creates a start menu button
                // when we enter the menu scene.
                ecs.entity("Start Button")
                    .set(components::Button{ "Play the Game!" })
                    .set(components::Position{ 50.0, 50.0 })
                    .child_of(scene);

                ecs.set_pipeline(ecs.get<scenes::MainMenu>()->pipeline);
            });

        // Observer to call scene change logic for scene::Level1 when added to the ActiveScene.
        world.observer<scenes::ActiveScene>("Scene Change to Game")
            .event(flecs::OnAdd)
            .second<scenes::Level>()
            .each([&](flecs::iter& it, size_t, scenes::ActiveScene) {
                std::cout << "\n>> ActiveScene has changed to `scene::Level`\n\n";

                flecs::world ecs = it.world();
                flecs::entity scene = ecs.component<scenes::SceneRoot>();

                reset_scene(ecs);

                // Creates a player character
                // when we enter the game scene.
                ecs.entity("Player")
                    .set(components::Character{})
                    .set(components::Health{ 2 })
                    .set(components::Position{ 0, 0 })
                    .child_of(scene);

                ecs.set_pipeline(ecs.get<scenes::Level>()->pipeline);
            });
    }
}
