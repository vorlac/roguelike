#pragma once

#include <flecs.h>

namespace rl::scenes
{

    using Pipeline = flecs::entity;

    struct ActiveScene
    {
    };

    struct SceneRoot
    {
    };

    struct MainMenu
    {
        Pipeline pipeline{};
    };

    struct Level
    {
        Pipeline pipeline{};
    };

    void init_level_scenes(flecs::world& world);

}
