#pragma once

#include <flecs.h>

namespace rl
{
    /// @brief Scene pipeline to order and schedule
    ///        systems for execution within the scene
    using Pipeline = flecs::entity;

    /// @brief Represents the
    ///        current scene
    struct ActiveScene
    {
    };

    /// @brief Parent for all entities
    ///        unique to the scene
    struct SceneRoot
    {
    };

    /// @brief Defines scene relationships
    ///        leveraging pipelines
    namespace scene
    {
        /// @brief Scene representing the
        ///        game's startup main menu
        struct MainMenu
        {
            Pipeline pipeline{};
        };

        /// @brief Scene owning all
        ///        level 1 entities
        struct Level
        {
            Pipeline pipeline{};
        };
    }
}
