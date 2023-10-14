#pragma once

#include <flecs.h>

namespace rl::observer
{
    // Removes all entities who are children of the current scene root.
    void reset_scene(flecs::world& world);

    void add_level_observers(flecs::world& world);

}
