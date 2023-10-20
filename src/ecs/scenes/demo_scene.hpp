#pragma once

#include <flecs.h>

#include "core/ds/point.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/io.hpp"
#include "core/utils/time.hpp"
#include "ecs/components/character_components.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::scene
{
    namespace observer
    {
        auto demo_level_onadd(flecs::iter& it, size_t, scene::active)
        {
            log::info("=== scene::active has changed to scene::demo_level ===");

            flecs::world world = it.world();
            flecs::entity scene = world.component<scene::root>();

            scene::reset(world);

            const auto generate_world_entities = [&](uint32_t count) {
                const ds::position centroid{
                    static_cast<float>(raylib::GetScreenWidth()) / 2.0f,
                    static_cast<float>(raylib::GetScreenHeight()) / 2.0f,
                };

                raylib::SetRandomSeed(696969420);
                for (size_t i = 0; i < count; ++i)
                {
                    color rect_color{
                        rand_color(raylib::GetRandomValue(0, 100)),
                    };

                    ds::velocity velocity{
                        static_cast<float>(raylib::GetRandomValue(-5000, 5000) / 10.0),
                        static_cast<float>(raylib::GetRandomValue(-5000, 5000) / 10.0),
                    };

                    world.entity(fmt::format("Rect {}", i).data())
                        .set<component::position>({ centroid.x, centroid.y })
                        .set<component::velocity>({ velocity.x, velocity.y })
                        .set<component::style>({ rect_color });
                }

                return world.count<component::position>();
            };

            rl::timer timer{ "scene::demo init" };
            timer.measure(generate_world_entities, 10000);
            world.set_pipeline(world.get<scene::demo_level>()->pipeline);
        }
    }

    auto init_demo_scene(flecs::world& world)
    {
        // Each scene gets a pipeline that
        // runs the associated systems plus
        // all other scene-agnostic systems.
        //
        // Use "without()" of the other
        // scenes so that we can run every
        // system that doesn't have a scene
        // attached to it.
        flecs::entity demo_scene = {
            world.pipeline()
                .with(flecs::System)          // has system
                .without<scene::main_menu>()  // doesn't have a menu
                .build()                      // construct
        };

        world.set<scene::demo_level>({ demo_scene });

        // scene::observer callback that implements scene change/creation
        // logic for scene::demo when it becomes the new scene::active.
        world.observer<scene::active>("active scene changed to scene::demo_level")
            .second<scene::demo_level>()
            .event(flecs::OnAdd)
            .each(scene::observer::demo_level_onadd);
    }
}
