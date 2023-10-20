#pragma once

#include <flecs.h>

#include "core/ds/dimensions.hpp"
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

        static auto init(flecs::world& world, ds::dimensions<int32_t> render_window_size)
        {
            world.set<benchmark_scene>({
                world.pipeline()
                    .with(flecs::System)
                    .without<main_menu_scene>()
                    .write()  //
                    .build()  //
            });

            uint64_t update_calls = 0;
            static const auto window_size{ render_window_size };
            static constexpr ds::dimensions rect_size{
                .width = 10,
                .height = 10,
            };

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

            world.system<component::position, component::velocity>("Movement")
                .kind(flecs::OnUpdate)
                .interval(1.0f / 120.0f)
                .iter([&](flecs::iter& it, component::position* p, component::velocity* v) {
                    const float delta_time{ it.delta_system_time() };

                    if (++update_calls % 60 == 0)
                    {
                        log::info(
                            "delta time: [{:>10.6f} ms]  update movment: (x:{:>4.3f}, y:{:<4.3f})",
                            delta_time * 1000.0, v->x * delta_time, v->y * delta_time);
                    }

                    for ([[maybe_unused]] const auto i : it)
                    {
                        p->x += v->x * delta_time;
                        p->y += v->y * delta_time;

                        if (left_right_collision(*p))
                            v->x = -v->x;
                        if (top_bottom_collision(*p))
                            v->y = -v->y;

                        ++v;
                        ++p;
                    }
                });

            {
                raylib::BeginDrawing();
                raylib::ClearBackground(color::lightgray);

                world.system<const component::position, const component::style>("Render")
                    .kind(flecs::PostUpdate)
                    .each([&](const component::position& p, const component::style& s) {
                        raylib::DrawRectangle(
                            static_cast<int32_t>(p.x) - static_cast<int32_t>(rect_size.width / 2.0),
                            static_cast<int32_t>(p.y) - static_cast<int32_t>(rect_size.height / 2.0),
                            static_cast<int32_t>(rect_size.width),
                            static_cast<int>(rect_size.height), s.color);
                    });

                raylib::DrawRectangle(0, 0, 95, 40, color::black);
                raylib::DrawFPS(10, 10);
                raylib::EndDrawing();
            }

            world.observer<scene::active>("active scene changed to scene::benchmark")
                .second<benchmark_scene>()
                .event(flecs::OnAdd)
                .each(observer::onadd_benchmark_scene);
        }

        scene::pipeline pipeline{};
    };
}
