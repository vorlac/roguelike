
#include "ecs/scenes.hpp"

namespace rl::scenes
{
    void init_level_scenes(flecs::world& world)
    {
        // // Can only have one active scene in a game at a time.
        // world.component<scenes::ActiveScene>().add(flecs::Exclusive);

        // Each scene gets a pipeline that runs the associated
        // systems plus all other scene-agnostic systems.
        //
        // Use "without()" of the other scenes so that we can run
        // every system that doesn't have a scene attached to it.

        // clang-format off
        flecs::entity menu_scene{
            world.pipeline()
                .with(flecs::System)
                .without<scenes::Level>()
                .build()
        };

        flecs::entity game_scene{
            world.pipeline()
                .with(flecs::System)
                .without<scenes::MainMenu>()
                .build()
        };

        // Set pipeline entities on the scenes to easily find them later with get().
        world.set<scenes::MainMenu>({ menu_scene });
        world.set<scenes::Level>({ game_scene });
        // clang-format on
    }
}
