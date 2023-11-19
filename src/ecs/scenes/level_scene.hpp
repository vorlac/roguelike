#pragma once

#include <flecs.h>

#include "ecs/components/character_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "utils/io.hpp"

namespace rl::scene {
    struct level1
    {
        scene::pipeline pipeline;
    };

    void scene_init_level1(flecs::iter& it, size_t, scene::active)
    {
        log::info("=== ActiveScene has changed to scene::level1");

        flecs::world world = it.world();
        flecs::entity scene = world.component<scene::root>();

        scene::reset(world);
        world.entity("Player")
            .set(component::character{})
            .set(component::health{ 2 })
            .set(component::position{ 0, 0 })
            .child_of(scene);

        world.set_pipeline(world.get<scene::benchmark_scene>()->pipeline);
    }
}
