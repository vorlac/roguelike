#pragma once

#include <flecs.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/input/input.hpp"
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

                flecs::world world = it.world();
                flecs::entity scene = world.component<scene::root>();

                scene::reset(world);

                raylib::SetRandomSeed(2147483647);

                const ds::position centroid{
                    static_cast<float>(raylib::GetScreenWidth()) / 2.0f,
                    static_cast<float>(raylib::GetScreenHeight()) / 2.0f,
                };

                constexpr size_t count = 25000;
                for (size_t i = 0; i < count; ++i)
                {
                    color rect_color{
                        rand_color(raylib::GetRandomValue(0, 100)),
                    };

                    ds::velocity velocity{
                        static_cast<float>(raylib::GetRandomValue(-1000, 1000) / 10.0),
                        static_cast<float>(raylib::GetRandomValue(-1000, 1000) / 10.0),
                    };

                    world.entity(fmt::format("Rect {}", i).data())
                        .set<component::position>({ centroid.x, centroid.y })
                        .set<component::velocity>({ velocity.x, velocity.y })
                        .set<component::style>({ rect_color });
                }

                world.set_pipeline(world.get<benchmark_scene>()->pipeline);
            }
        };

        struct system
        {
            static void define_rect_movement(flecs::world& world, ds::dimensions<int32_t> window_rect)
            {
                uint64_t update_calls = 0;
                static const auto window_size{ window_rect };

                auto top_bottom_collision = [&](const component::position& pos) {
                    bool top_collision = pos.y - (rect_size.height / 2.0) <= 0.0;
                    bool bottom_collision = pos.y + (rect_size.height / 2.0) >= window_size.height;
                    return top_collision || bottom_collision;
                };

                auto left_right_collision = [&](const component::position& pos) {
                    bool left_collision = pos.x - (rect_size.width / 2.0) <= 0.0;
                    bool right_collision = pos.x + (rect_size.width / 2.0) >= window_size.width;
                    return left_collision || right_collision;
                };

                world.system<component::position, component::velocity>("Rect Movement")
                    .kind(flecs::OnUpdate)
                    .interval(1.0f / 120.0f)
                    .iter([&](flecs::iter& it, component::position* pos, component::velocity* vel) {
                        const float delta_time = it.delta_system_time();
                        if (++update_calls % 120 == 0)
                        {
                            rl::log::info(
                                "delta time: [{:>10.6f} ms]  update movment: (x:{:>4.3f}, y:{:<4.3f})",
                                delta_time * 1000.0f, vel->x * delta_time, vel->y * delta_time);
                        }

                        for (const auto i : it)
                        {
                            pos->x += vel->x * delta_time;
                            pos->y += vel->y * delta_time;

                            if (left_right_collision(*pos))
                                vel->x = -vel->x;
                            if (top_bottom_collision(*pos))
                                vel->y = -vel->y;

                            ++vel;
                            ++pos;
                        }
                    });
            }

            static void define_player_movement(flecs::world&)
            {
                // world.system<const component::character, component::velocity>("Player Movement")
                //     .kind(flecs::OnUpdate)
                //     .interval(1.0f / 120.0f)
                //     .each([](flecs::entity& e, const component::character&, component::velocity&
                //     vel) {
                //         static std::vector<input::GameplayAction> inputs =
                //     });
            }

            static void define_entity_rendering(flecs::world& world)
            {
                world.system<const component::position, const component::style>("Render")
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
                    .each([&](const component::position& p, const component::style& s) {
                        raylib::DrawRectangle(
                            static_cast<int32_t>(p.x) - static_cast<int32_t>(rect_size.width / 2.0),
                            static_cast<int32_t>(p.y) - static_cast<int32_t>(rect_size.height / 2.0),
                            static_cast<int32_t>(rect_size.width),
                            static_cast<int32_t>(rect_size.height), s.color);
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

            static void init_systems(flecs::world& world, ds::dimensions<int32_t> window_rect)
            {
                define_player_movement(world);
                define_rect_movement(world, window_rect);
                define_entity_rendering(world);
                define_entity_timeout(world);
            }
        };

    public:
        static auto init(flecs::world& world, ds::dimensions<int32_t> window_rect)
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

    private:
        static constexpr ds::dimensions rect_size{
            .width = 10,
            .height = 10,
        };
    };
}
