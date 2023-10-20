#pragma once

#include <flecs.h>

#include "core/utils/io.hpp"
#include "core/utils/time.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/components/ui_components.hpp"
#include "ecs/scenes/scene_types.hpp"

namespace rl::scene
{
    namespace observer
    {
        auto main_menu_onadd(flecs::iter& it, size_t, scene::active)
        {
            log::info("=== scene::active has changed to scene::main_menu ===");

            flecs::world world = it.world();
            flecs::entity scene = world.component<scene::root>();

            scene::reset(world);

            const auto generate_main_menu = [&] {
                // Creates a start menu button
                // when we enter the menu scene.
                world.entity("Start Game Button")
                    .set(component::button{ "Start Game" })
                    .set(component::position{ 50.0, 50.0 })
                    .child_of(scene);

                world.set_pipeline(world.get<scene::main_menu>()->pipeline);
                return true;
            };

            rl::timer init_timer{ "scene::main_menu init" };
            init_timer.measure(generate_main_menu);

            world.set_pipeline(world.get<scene::main_menu>()->pipeline);
        }
    }

    auto init_main_menu_scene(flecs::world& world)
    {
        // Each scene gets a pipeline that
        // runs the associated systems plus
        // all other scene-agnostic systems.
        //
        // Use "without()" of the other
        // scenes so that we can run every
        // system that doesn't have a scene
        // attached to it.
        flecs::entity menu_scene = {
            world.pipeline()
                .with(flecs::System)           // has system
                .without<scene::demo_level>()  // doesn't have the level_demo scene component
                .build()                       // construct
        };

        world.set<scene::main_menu>({ menu_scene });

        // scene::observer callback that implements scene change/creation
        // logic for scene::demo when it becomes the new scene::active.
        world.observer<scene::active>("active scene changed to scene::main_menu")
            .second<scene::main_menu>()
            .event(flecs::OnAdd)
            .each(scene::observer::main_menu_onadd);
    }
}
