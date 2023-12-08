#pragma once

#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include <flecs.h>
#include <fmt/format.h>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "ecs/components/character_components.hpp"
#include "ecs/components/kinematic_components.hpp"
#include "ecs/components/projectile_components.hpp"
#include "ecs/components/style_components.hpp"
#include "ecs/components/transform_components.hpp"
#include "ecs/scenes/scene_types.hpp"
#include "sdl/defs.hpp"
#include "sdl/pixel_data.hpp"
#include "sdl/renderer.hpp"
#include "sdl/renderer_opengl.hpp"
#include "sdl/surface.hpp"
#include "sdl/tests/data/icon.hpp"
#include "sdl/texture.hpp"
#include "sdl/window.hpp"
#include "utils/assert.hpp"
#include "utils/conversions.hpp"
#include "utils/io.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_rwops.h>
SDL_C_LIB_END

namespace rl::scene {
    struct benchmark
    {
        constexpr static inline size_t ENTITY_COUNT = 50000;

        struct observer
        {
            static auto onadd_benchmark_scene(flecs::iter& it, size_t, scene::active)
            {
                log::info("=== scene::active has changed to scene::demo_level ===");

                flecs::world world = it.world();
                flecs::entity scene = world.component<scene::root>();
                scene::reset(world);

                auto rect = m_renderer->get_viewport();
                const ds::point<f32> centroid = rect.centroid();

                srand((u32)time(nullptr));
                auto rect_color = ds::color<u8>{
                    static_cast<u8>(rand() % 128),
                    static_cast<u8>(rand() % 128),
                    static_cast<u8>(rand() % 128),
                };

                for (size_t i = 0; i < benchmark::ENTITY_COUNT; ++i)
                {
                    u32 xv = rand() % 2000;
                    u32 yv = rand() % 2000;

                    ds::vector2<f32> velocity{
                        static_cast<f32>((xv - 1000.0) / 10.0),
                        static_cast<f32>((yv - 1000.0) / 10.0),
                    };
                    rect_color = ds::color<u8>{
                        static_cast<u8>(rand() % 128),
                        static_cast<u8>(rand() % 128),
                        static_cast<u8>(rand() % 128),
                    };
                    world.entity(fmt::format("Rect {}", i).data())
                        .set<component::position>({ centroid.x, centroid.y })
                        .set<component::velocity>({ velocity.x, velocity.y })
                        .set<component::style>({ rect_color })
                        .set<component::scale>({ 1.0f })
                        .child_of(scene);
                }

                world.entity("Player")
                    .set<component::position>({ centroid.x, centroid.y })
                    .set<component::velocity>({ 0.0f, 0.0f })
                    .set<component::style>({ rect_color })
                    .set<component::character>({ true })
                    .set<component::scale>({ 5.0f })
                    .child_of(scene);

                world.set_pipeline(world.get<benchmark_scene>()->pipeline);
            }
        };

        struct system
        {
            // static void define_player_movement(flecs::world& world)
            // {
            //     static float delta_time{ 0.0f };
            //     constexpr float target_speed{ 100.0f };
            //     static std::vector<input::GameplayAction> input_actions{};

            //     world.system<component::character, component::velocity>("Player Movement")
            //         .kind(flecs::OnUpdate)
            //         .run([](flecs::iter_t* it) {
            //             delta_time = it->delta_system_time;
            //             input_actions = m_input.active_game_actions();
            //             while (ecs_iter_next(it))
            //                 it->callback(it);
            //         })
            //         .interval(1.0f / 120.0f)
            //         .each([](flecs::entity, component::character&, component::velocity& vel) {
            //             std::vector<input::GameplayAction> inputs{
            //                 m_input.active_game_actions(),
            //             };

            //             if (inputs.empty())
            //                 return;

            //             for (const auto action : inputs)
            //             {
            //                 switch (action)
            //                 {
            //                     case input::GameplayAction::None:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::Dash:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::Shoot:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::UseItem:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::NextWeapon:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::PrevWeapon:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::ToggleDebugInfo:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::RotateUp:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::RotateDown:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::RotateLeft:
            //                         [[fallthrough]];
            //                     case input::GameplayAction::RotateRight:
            //                         break;

            //                     case input::GameplayAction::MoveLeft:
            //                         vel.x -= 1.0f * target_speed * delta_time;
            //                         break;
            //                     case input::GameplayAction::MoveRight:
            //                         vel.x += 1.0f * target_speed * delta_time;
            //                         break;
            //                     case input::GameplayAction::MoveUp:
            //                         vel.y -= 1.0f * target_speed * delta_time;
            //                         break;
            //                     case input::GameplayAction::MoveDown:
            //                         vel.y += 1.0f * target_speed * delta_time;
            //                         break;
            //                 }
            //             }
            //         });
            // }

            static void define_rect_movement(flecs::world& world, const ds::dims<i32>& window_rect)
            {
                const static auto window_size = window_rect;

                constexpr static auto top_bottom_collision = [&](const component::position& pos) {
                    bool top_collision = pos.y - (rect_size.height / 2.0f) <= 0.0f;
                    bool bottom_collision = pos.y + (rect_size.height / 2.0f) >=
                                            cast::to<float>(window_size.height);
                    return top_collision || bottom_collision;
                };

                constexpr static auto left_right_collision = [&](const component::position& pos) {
                    bool left_collision = pos.x - (rect_size.width / 2.0f) <= 0.0f;
                    bool right_collision = pos.x + (rect_size.width / 2.0f) >=
                                           cast::to<float>(window_size.width);
                    return left_collision || right_collision;
                };

                world.system<component::position, component::velocity>("Rect Movement")
                    .kind(flecs::OnUpdate)
                    .multi_threaded(true)
                    .interval(1.0f / 120.0f)
                    .run([](flecs::iter_t* it) {
                        m_delta_time = it->delta_system_time;
                        while (ecs_iter_next(it))
                            it->callback(it);
                    })
                    .each([](flecs::entity, component::position& pos, component::velocity& vel) {
                        pos.x += vel.x * static_cast<f32>(m_delta_time);
                        pos.y += vel.y * static_cast<f32>(m_delta_time);

                        if (left_right_collision(pos))
                            vel.x = -vel.x;
                        if (top_bottom_collision(pos))
                            vel.y = -vel.y;
                    });
            }

            constexpr static inline bool USE_RANDOM_COLORS = false;
            constexpr static inline ds::dims<f32> RECT_SIZE = {
                USE_RANDOM_COLORS ? ds::dims<f32>{ 10.0f, 10.0f } : ds::dims<f32>{ 20.0f, 20.0f }
            };

            static void define_entity_rendering(flecs::world& ecs,
                                                std::unique_ptr<sdl::Window>& window)
            {
                //*m_sprite = std::move(sprite);

                m_renderer = window->renderer().get();
                m_renderer->set_draw_blend_mode(USE_RANDOM_COLORS ? sdl::Renderer::blend_mode::Blend
                                                                  : sdl::Renderer::blend_mode::Mod);

                static u32 count = 0;
                static ds::color<u8> c = { 100, 200, 100, 75 };
                static std::vector<ds::rect<f32>> rects = {};
                if constexpr (!USE_RANDOM_COLORS)
                    rects.reserve(benchmark::ENTITY_COUNT);

                static std::vector<std::pair<ds::rect<f32>, ds::color<u8>>> rect_colors = {};
                if constexpr (USE_RANDOM_COLORS)
                    rect_colors.reserve(benchmark::ENTITY_COUNT);

                const auto window_context_size{ m_renderer->get_output_size() };

                sdl::Surface surface{
                    window_context_size.width,
                    window_context_size.height,
                    sdl::PixelData::format::RGB24,
                };

                // static sdl::Texture texture1{ window.renderer(), surface };
                // static sdl::Texture texture2{ window.renderer(), surface };

                ecs.system<const component::position, const component::style, const component::scale>(
                       "Render Rects")
                    .kind(flecs::PostUpdate)
                    .multi_threaded(true)
                    .run([](flecs::iter_t* it) {
                        m_renderer->clear({ 100, 100, 100, 175 });
                        USE_RANDOM_COLORS ? rect_colors.clear()  //
                                          : rects.clear();

                        static i16 ga = 1;
                        static i16 ba = 1;

                        if ((++count % 120) == 0)
                        {
                            if (c.g > 250 || c.g < 105)
                                ga *= -1;
                            if (c.b > 250 || c.b < 105)
                                ba *= -1;

                            c.g += static_cast<u8>(ga);
                            c.b += static_cast<u8>(ba);
                        }

                        while (ecs_iter_next(it))
                            it->callback(it);

                        // USE_RANDOM_COLORS
                        //? m_renderer->fill_rects(rect_colors)  //
                        //: m_renderer->fill_rects(rects, c);

                        m_renderer->present();
                    })
                    .each(
                        [&](const rl::component::position& position,
                            const rl::component::style& rect_color,
                            const rl::component::scale& size_scale)  //
                        {
                            if constexpr (USE_RANDOM_COLORS)
                                rect_colors.emplace_back(
                                    std::forward<const ds::rect<f32>>(
                                        { position, system::RECT_SIZE * size_scale.factor }),
                                    std::forward<const ds::color<u8>>(rect_color.color));

                            if constexpr (!USE_RANDOM_COLORS)
                                rects.emplace_back(position, system::RECT_SIZE * size_scale.factor);
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

            static sdl::Texture create_texture(std::shared_ptr<sdl::Renderer> renderer,
                                               std::vector<u8>& data, ds::dims<i32>& size)
            {
                SDL3::SDL_RWops* src = SDL3::SDL_RWFromConstMem(data.data(), data.size());
                if (src != nullptr)
                {
                    /* Treat white as transparent */
                    ds::color<u8> c{ 255, 255, 255 };
                    sdl::Surface surface = SDL3::SDL_LoadBMP_RW(src, SDL_TRUE);
                    if (surface.is_valid())
                    {
                        surface.set_color_key(true, c.rgb(surface.get_format_full()));
                        sdl::Texture texture{ *renderer, surface };
                        size = surface.size();
                        return texture;
                    }
                }

                runtime_assert(false, "failed to create texture");
                return sdl::Texture{ nullptr };
            }

            static void init_systems(flecs::world& world, std::unique_ptr<sdl::Window>& window)
            {
                ds::dims<i32> sprite_size{ 0, 0 };
                std::vector<u8> icon_data = { icon_bmp, icon_bmp + icon_bmp_len };
                // auto sprite = create_texture(window.renderer(), icon_data, sprite_size);
                // runtime_assert(sprite.is_valid(), "failed to load sprite");
                define_rect_movement(world, window->get_size());
                define_entity_rendering(world, window);
                define_entity_timeout(world);
            }
        };

    public:
        static auto init(flecs::world& world, std::unique_ptr<sdl::Window>& window)
        {
            world.set<benchmark_scene>({
                world.pipeline()
                    .with(flecs::System)
                    .without<main_menu_scene>()  //
                    .build()                     //
            });

            // render_size = window.get_render_size();
            system::init_systems(world, window);

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

        static bool deinit()
        {
            if (m_sprite != nullptr)
            {
                delete m_sprite;
                m_sprite = nullptr;
            }
            return m_sprite == nullptr;
        }

    public:
        scene::pipeline pipeline{};

        // static inline rl::Input m_input{};
        static inline thread_local f64 m_delta_time{ 0.0f };
        static inline thread_local i64 m_update_calls{ 0 };
        constexpr static inline ds::dims<i32> rect_size{ 10, 10 };
        static inline ds::dims<i32> render_size{ 0, 0 };
        static inline sdl::RendererGL* m_renderer{ nullptr };
        static inline sdl::Texture* m_sprite{ new sdl::Texture{} };
    };
}
