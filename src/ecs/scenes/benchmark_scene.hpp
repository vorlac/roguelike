#pragma once

#include <vector>
#include <flecs.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/input/input.hpp"
#include "core/input/keymap.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/io.hpp"
#include "core/utils/time.hpp"
#include "ecs/components/character_components.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/projectile_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "thirdparty/raylib.hpp"

namespace rl::scene
{
    struct benchmark
    {
        struct observer
        {
            static auto onadd_benchmark_scene(flecs::iter& it, size_t, scene::active)
            {
                log::info("=== scene::active has changed to scene::demo_level ===");

                flecs::world world  = it.world();
                flecs::entity scene = world.component<scene::root>();

                scene::reset(world);

                raylib::SetRandomSeed(2147483647);

                const ds::point<f32> centroid{
                    static_cast<f32>(raylib::GetScreenWidth()) / 2.0f,
                    static_cast<f32>(raylib::GetScreenHeight()) / 2.0f,
                };

                constexpr size_t count = 25000;
                for (size_t i = 0; i < count; ++i)
                {
                    color rect_color{
                        rand_color(raylib::GetRandomValue(0, 100)),
                    };

                    ds::velocity<f32> velocity{
                        static_cast<f32>(raylib::GetRandomValue(-1000, 1000) / 10.0),
                        static_cast<f32>(raylib::GetRandomValue(-1000, 1000) / 10.0),
                    };

                    world.entity(fmt::format("Rect {}", i).data())
                        .set<component::position>({ .x = centroid.x, .y = centroid.y })
                        .set<component::velocity>({ .x = velocity.x, .y = velocity.y })
                        .set<component::style>({ .color = rect_color })
                        .set<component::scale>({ .factor = 1.0f })
                        .child_of(scene);
                }

                world.entity("Player")
                    .set<component::position>({ .x = centroid.x, .y = centroid.y })
                    .set<component::velocity>({ .x = 0.0f, .y = 0.0f })
                    .set<component::style>({ .color = color::orange })
                    .set<component::character>({ .alive = true })
                    .set<component::scale>({ .factor = 5.0f })
                    .child_of(scene);

                world.set_pipeline(world.get<benchmark_scene>()->pipeline);
            }
        };

        struct system
        {
            static void define_rect_movement(flecs::world& world, ds::dimensions<i32> window_rect)
            {
                const static ds::dimensions<i32> window_size{ window_rect };

                static auto top_bottom_collision = [](const component::position& pos) {
                    bool top_collision    = pos.y - (rect_size.height / 2.0f) <= 0.0f;
                    bool bottom_collision = pos.y + (rect_size.height / 2.0f) >=
                                            static_cast<float>(window_size.height);
                    return top_collision || bottom_collision;
                };

                static auto left_right_collision = [](const component::position& pos) {
                    bool left_collision  = pos.x - (rect_size.width / 2.0f) <= 0.0f;
                    bool right_collision = pos.x + (rect_size.width / 2.0f) >=
                                           static_cast<float>(window_size.width);
                    return left_collision || right_collision;
                };

                world.system<component::position, component::velocity>("Rect Movement")
                    .kind(flecs::OnUpdate)
                    .interval(1.0f / 120.0f)
                    .run([](flecs::iter_t* it) {
                        if (++m_update_calls % 120 == 0)
                            rl::log::info("delta time: [{:>10.6f} ms]", m_delta_time);

                        m_delta_time = it->delta_system_time;
                        while (ecs_iter_next(it))
                            it->callback(it);
                    })
                    .each([](flecs::entity, component::position& pos, component::velocity& vel) {
                        pos.x += vel.x * m_delta_time;
                        pos.y += vel.y * m_delta_time;

                        if (left_right_collision(pos))
                            vel.x = -vel.x;
                        if (top_bottom_collision(pos))
                            vel.y = -vel.y;
                    });
            }

            static void define_player_movement(flecs::world& world)
            {
                static float delta_time{ 0.0f };
                constexpr float target_speed{ 100.0f };
                static std::vector<input::GameplayAction> input_actions{};

                world.system<component::character, component::velocity>("Player Movement")
                    .kind(flecs::OnUpdate)
                    .run([](flecs::iter_t* it) {
                        delta_time    = it->delta_system_time;
                        input_actions = m_input.active_game_actions();
                        while (ecs_iter_next(it))
                            it->callback(it);
                    })
                    .interval(1.0f / 120.0f)
                    .each([](flecs::entity, component::character&, component::velocity& vel) {
                        std::vector<input::GameplayAction> inputs{
                            m_input.active_game_actions(),
                        };

                        if (inputs.empty())
                            return;

                        for (const auto action : inputs)
                        {
                            switch (action)
                            {
                                case input::GameplayAction::None:
                                    [[fallthrough]];
                                case input::GameplayAction::Dash:
                                    [[fallthrough]];
                                case input::GameplayAction::Shoot:
                                    [[fallthrough]];
                                case input::GameplayAction::UseItem:
                                    [[fallthrough]];
                                case input::GameplayAction::NextWeapon:
                                    [[fallthrough]];
                                case input::GameplayAction::PrevWeapon:
                                    [[fallthrough]];
                                case input::GameplayAction::ToggleDebugInfo:
                                    [[fallthrough]];
                                case input::GameplayAction::RotateUp:
                                    [[fallthrough]];
                                case input::GameplayAction::RotateDown:
                                    [[fallthrough]];
                                case input::GameplayAction::RotateLeft:
                                    [[fallthrough]];
                                case input::GameplayAction::RotateRight:
                                    break;

                                case input::GameplayAction::MoveLeft:
                                    vel.x -= 1.0f * target_speed * delta_time;
                                    break;
                                case input::GameplayAction::MoveRight:
                                    vel.x += 1.0f * target_speed * delta_time;
                                    break;
                                case input::GameplayAction::MoveUp:
                                    vel.y -= 1.0f * target_speed * delta_time;
                                    break;
                                case input::GameplayAction::MoveDown:
                                    vel.y += 1.0f * target_speed * delta_time;
                                    break;
                            }
                        }
                    });
            }

            static void define_entity_rendering(flecs::world& world)
            {
                world
                    .system<const component::position, const component::style, const component::scale>(
                        "Render Rects")
                    .kind(flecs::PostUpdate)
                    .run([](flecs::iter_t* it) {
                        raylib::BeginDrawing();
                        raylib::ClearBackground(color::lightgray);

                        while (ecs_iter_next(it))
                            it->callback(it);

                        raylib::DrawRectangle(0, 0, 95, 40, color::black);
                        raylib::DrawFPS(10, 10);
                        raylib::EndDrawing();
                    })
                    .each([&](const component::position& p, const component::style& c,
                              const component::scale& s) {
                        raylib::DrawRectangle(
                            static_cast<i32>(p.x) - static_cast<i32>(rect_size.width / 2.0f),
                            static_cast<i32>(p.y) - static_cast<i32>(rect_size.height / 2.0f),
                            static_cast<i32>(rect_size.width * s.factor),
                            static_cast<i32>(rect_size.height * s.factor), c.color);
                    });
            }

            static void define_entity_timeout(flecs::world& world)
            {
                world.system<component::timeout>().each(
                    [](flecs::iter& it, size_t index, component::timeout& t) {
                        t.ttl -= it.delta_time();
                        if (t.ttl <= 0)
                        {
                            flecs::entity e{ it.entity(index) };
                            log::info("{} TTL expired, deleting", e.name());
                            e.destruct();
                        }
                    });
            }

            static void init_systems(flecs::world& world, ds::dimensions<i32> window_rect)
            {
                define_rect_movement(world, window_rect);
                define_player_movement(world);
                define_entity_rendering(world);
                define_entity_timeout(world);
            }
        };

    public:
        static auto init(flecs::world& world, ds::dimensions<i32> window_rect)
        {
            world.set<benchmark_scene>({
                world.pipeline()
                    .with(flecs::System)
                    .without<main_menu_scene>()  //
                    .build()                     //
            });

            system::init_systems(world, window_rect);

            world.observer<scene::active>("active scene changed to scene::benchmark")
                .second<benchmark_scene>()
                .event(flecs::OnAdd)
                .each(observer::onadd_benchmark_scene);

            // triggers when entity is deleted
            world.observer<component::timeout>()
                .event(flecs::OnRemove)
                .each([](flecs::entity e, component::timeout&) {
                    log::info("Entity deleted: {}", e);
                });
        }

    public:
        scene::pipeline pipeline{};

        static inline input::Input m_input{};
        static inline thread_local float m_delta_time{ 0.0f };
        static inline thread_local int64_t m_update_calls{ 0 };
        static constexpr inline ds::dimensions<i32> rect_size{ 10, 10 };
    };
}
