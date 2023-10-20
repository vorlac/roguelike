#pragma once

#include <flecs.h>

namespace rl::scene
{
    using pipeline = flecs::entity;

    struct active
    {
    };

    struct root
    {
    };

    struct main_menu_scene
    {
        scene::pipeline pipeline{};
    };

    struct benchmark_scene
    {
        scene::pipeline pipeline{};
    };

    // Removes all entities who are children of the current scene root.
    void reset(flecs::world& world)
    {
        world.defer_begin();
        world.delete_with(flecs::ChildOf, world.entity<scene::root>());
        world.defer_end();
    }

    template <typename TScene>
    void set_active(flecs::world& world)
    {
        world.add<scene::active, TScene>();
    }
}
