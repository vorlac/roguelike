#pragma once

#include <string>
#include <flecs.h>

#include "core/application.hpp"
#include "ecs/components/character_components.hpp"
#include "ecs/components/scene_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/components/ui_components.hpp"
#include "ecs/scenes/observables.hpp"
#include "ecs/scenes/scene_types.hpp"

namespace rl
{
    class Game : public Application
    {
    public:
        Game() noexcept
        {
            this->init_scenes();
            this->init_systems();
        }

        ~Game() noexcept
        {
            m_world.quit();
        }

        inline bool run() const
        {
            // start in the menu scene
            m_world.add<ActiveScene, scene::MainMenu>();

            // switch to game scene
            // m_world.add<ActiveScene, scene::Level1>();

            while (!this->should_quit())
            {
                this->update();
                this->render();
            }

            return 0;
        }

        // Progress a world.
        // This operation progresses the world by running all systems that are both
        // enabled and periodic on their matching entities.
        inline void update() const
        {
            // An application can pass a delta_time into the function, which is the time
            // passed since the last frame. This value is passed to systems so they can
            // update entity values proportional to the elapsed time since their last
            // invocation.
            //
            // When an application passes 0 to delta_time, ecs_progress will automatically
            // measure the time passed since the last frame. If an application does not uses
            // time management, it should pass a non-zero value for delta_time (1.0 is
            // recommended). That way, no time will be wasted measuring the time.
            m_world.progress(this->delta_time());
        }

        inline bool render() const
        {
            static std::string text{};
            text = fmt::format("FPS: {}", this->framerate());

            m_window.render([&] {
                ::ClearBackground(RAYWHITE);
                ::DrawText(text.data(), 190, 200, 20, GRAY);
            });

            return true;
        }

        /// @brief determines if the main run() loop should terminate
        /// @return true if window has been closed or quit has been signaled from flecs
        inline bool should_quit() const
        {
            return m_world.should_quit() || m_window.should_close();
        }

        /// @brief Removes all entities who are children of the current scene root.
        /// @note should use defer_begin() / defer_end()
        static inline void reset_scene(flecs::world& ecs_world)
        {
            ecs_world.defer_begin();
            ecs_world.delete_with(flecs::ChildOf, ecs_world.entity<component::SceneRoot>());
            ecs_world.defer_end();
        }

        void init_scenes() const
        {
            // Can only have one active scene in a game at a time.
            m_world.component<ActiveScene>().add(flecs::Exclusive);

            // Each scene gets a pipeline that runs the associated
            // systems plus all other scene-agnostic systems.
            //
            // Use "without()" of the other scenes so that we can run
            // every system that doesn't have a scene attached to it.

            // clang-format off
            flecs::entity menu_scene{
                m_world.pipeline()
                       .with(flecs::System)
                       .without<scene::Level1>()
                       .build()
            };

            flecs::entity game_scene{
                m_world.pipeline()
                       .with(flecs::System)
                       .without<scene::MainMenu>()
                       .build()
            };
            // clang-format on

            // Set pipeline entities on the scenes to easily find them later with get().
            m_world.set<scene::MainMenu>({ menu_scene });
            m_world.set<scene::Level1>({ game_scene });

            // Observer to call scene change logic for scene::MainMenu when added to the ActiveScene.
            m_world.observer<ActiveScene>("Scene Change to Menu")
                .event(flecs::OnAdd)
                .second<scene::MainMenu>()
                .each([](flecs::iter& it, size_t, ActiveScene) {
                    std::cout << "\n>> ActiveScene has changed to `scene::MainMenu`\n\n";

                    flecs::world ecs = it.world();
                    flecs::entity scene = ecs.component<component::SceneRoot>();

                    /**
                     * @brief Removes all entities who are children of the current scene root.
                     * @note should use defer_begin() / defer_end()
                     * */
                    Game::reset_scene(ecs);

                    // Creates a start menu button
                    // when we enter the menu scene.
                    ecs.entity("Start Button")
                        .set(component::Button{ "Play the Game!" })
                        .set(component::Position{ 50.0, 50.0 })
                        .child_of(scene);

                    ecs.set_pipeline(ecs.get<scene::MainMenu>()->pipeline);
                });

            // Observer to call scene change logic for scene::Level1 when added to the ActiveScene.
            m_world.observer<ActiveScene>("Scene Change to Game")
                .event(flecs::OnAdd)
                .second<scene::Level1>()
                .each([&](flecs::iter& it, size_t, ActiveScene) {
                    std::cout << "\n>> ActiveScene has changed to `scene::Level1`\n\n";

                    flecs::world ecs = it.world();
                    flecs::entity scene = ecs.component<component::SceneRoot>();

                    Game::reset_scene(ecs);

                    // Creates a player character
                    // when we enter the game scene.
                    ecs.entity("Player")
                        .set(component::Character{})
                        .set(component::Health{ 2 })
                        .set(component::Position{ 0, 0 })
                        .child_of(scene);

                    ecs.set_pipeline(ecs.get<scene::Level1>()->pipeline);
                });
        }

        void init_systems() const
        {
            // Will run every time regardless of the current scene we're in.
            m_world.system<const component::Position>("Print Position")
                .each([](flecs::entity e, const component::Position& p) {
                    // Prints out the position of the entity.
                    std::cout << e.name() << ": {" << p.x << ", " << p.y << "}\n";
                });

            // Will only run when the game scene is currently active.
            m_world.system<component::Health>("Characters Lose Health")
                .kind<scene::Level1>()
                .each([](component::Health& h) {
                    // Prints out the character's health and then decrements it by one.
                    std::cout << h.amount << " health remaining\n";
                    h.amount--;
                });

            // Will only run when the menu scene is currently active.
            m_world.system<const component::Button>("Print Menu Button Text")
                .kind<scene::MainMenu>()
                .each([](const component::Button& b) {
                    // Prints out the text of the menu button.
                    std::cout << "Button says \"" << b.text << "\"\n";
                });
        }

    private:
        flecs::world m_world{};
    };
}
