#pragma once

#include <flecs.h>

#include "core/utils/io.hpp"
#include "core/utils/time.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/components/ui_components.hpp"
#include "ecs/scenes/scene_types.hpp"

namespace rl::scene
{
    struct main_menu
    {
        struct observer
        {
            inline static auto main_menu_scene_onadd(flecs::iter& it, size_t, scene::active)
            {
                log::info("=== scene::active has changed to scene::main_menu ===");

                flecs::world world{ it.world() };
                flecs::entity scene{ world.component<scene::root>() };

                scene::reset(world);

                world.entity("Start Game Button")
                    .set(component::button{ "Start Game" })
                    .set(component::position{ 50.0, 50.0 })
                    .child_of(scene);

                world.set_pipeline(world.get<main_menu_scene>()->pipeline);
            }
        };

        inline static auto init(flecs::world& world)
        {
            world.set<main_menu_scene>({
                world.pipeline()
                    .with(flecs::System)         //
                    .without<benchmark_scene>()  //
                    .build()                     //
            });

            world.observer<scene::active>("active scene changed to scene::main_menu")
                .second<main_menu_scene>()
                .event(flecs::OnAdd)
                .each(observer::main_menu_scene_onadd);
        }
    };
}
