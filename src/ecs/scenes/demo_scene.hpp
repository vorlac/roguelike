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
    namespace observer
    {
        auto demo_level_onadd(flecs::iter& it, size_t, scene::active)
        {
            log::info("=== scene::active has changed to scene::demo_level ===");

            flecs::world world = it.world();
            flecs::entity scene = world.component<scene::root>();

            scene::reset(world);

            const auto generate_world_entities = [&](uint32_t count) {
                const ds::position centroid{
                    static_cast<float>(raylib::GetScreenWidth()) / 2.0f,
                    static_cast<float>(raylib::GetScreenHeight()) / 2.0f,
                };

                raylib::SetRandomSeed(696969420);
                for (size_t i = 0; i < count; ++i)
                {
                    color rect_color{
                        rand_color(raylib::GetRandomValue(0, 100)),
                    };

                    ds::velocity velocity{
                        static_cast<float>(raylib::GetRandomValue(-5000, 5000) / 10.0),
                        static_cast<float>(raylib::GetRandomValue(-5000, 5000) / 10.0),
                    };

                    world.entity(fmt::format("Rect {}", i).data())
                        .set<component::position>({ centroid.x, centroid.y })
                        .set<component::velocity>({ velocity.x, velocity.y })
                        .set<component::style>({ rect_color });
                }

                return world.count<component::position>();
            };

            rl::timer timer{ "scene::demo init" };
            timer.measure(generate_world_entities, 25000);
            world.set_pipeline(world.get<scene::demo_level>()->pipeline);
        }
    }

    auto init_demo_scene(flecs::world& world, ds::dimensions<int32_t> render_window_size)
    {
        // variables / lambdas captured by the update system
        static constexpr ds::dimensions rect_size{
            .width = 10,
            .height = 10,
        };

        static const auto window_size{ render_window_size };

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

        flecs::entity demo_scene = {
            world.pipeline()
                .with(flecs::System)
                .without<scene::main_menu>()  //
                .build()                      //
        };

        world.set<scene::demo_level>({ demo_scene });

        static std::atomic<uint64_t> update_calls = 0;
        static thread_local rl::timer delta_timer{ "delta_time" };

        world.system<component::position, component::velocity>("Movement")
            .kind(flecs::OnUpdate)
            .interval(1.0f / 120.0f)
            .iter([&](flecs::iter& it, component::position* p, component::velocity* v) {
                const float delta_time = it.delta_time();
                ++update_calls % 120 == 0       ? delta_timer.print_delta_time()
                : (update_calls + 1) % 120 == 0 ? delta_timer.delta_update()
                                                : (void)0;

                for ([[maybe_unused]] const auto i : it)
                {
                    p->x += v->x * delta_time;
                    p->y += v->y * delta_time;

                    if (left_right_collision(*p))
                        v->x = -v->x;
                    if (top_bottom_collision(*p))
                        v->y = -v->y;

                    ++v, ++p;
                }
            });

        raylib::BeginDrawing();
        raylib::ClearBackground(color::lightgray);
        world.system<const component::position, const component::style>("Render")
            .kind(flecs::PostUpdate)
            .each([&](const component::position& p, const component::style& s) {
                raylib::DrawRectangle(
                    static_cast<int32_t>(p.x) - static_cast<int32_t>(rect_size.width / 2.0),
                    static_cast<int32_t>(p.y) - static_cast<int32_t>(rect_size.height / 2.0),
                    static_cast<int32_t>(rect_size.width), static_cast<int>(rect_size.height),
                    s.color);
            });
        raylib::DrawRectangle(0, 0, 95, 40, color::black);
        raylib::DrawFPS(10, 10);
        raylib::EndDrawing();

        // scene::observer callback that implements scene change/creation
        // logic for scene::demo when it becomes the new scene::active.
        world.observer<scene::active>("active scene changed to scene::demo_level")
            .second<scene::demo_level>()
            .event(flecs::OnAdd)
            .each(scene::observer::demo_level_onadd);
    }
}
