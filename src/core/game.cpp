#include <chrono>
#include <fmt/format.h>
#include <fmt/printf.h>

#include "core/application.hpp"
#include "core/game.hpp"
#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ecs/components.hpp"
#include "ecs/observers.hpp"
#include "ecs/scenes.hpp"
#include "ecs/systems.hpp"
#include "utils/color.hpp"
#include "utils/time.hpp"

namespace rl
{
    namespace asdf
    {
        struct position
        {
            float x{ 0 };
            float y{ 0 };
        };

        struct velocity
        {
            float x{ 0 };
            float y{ 0 };
        };

        struct style
        {
            Color color{ rl::color::lime };
        };
    }

    Game::~Game()
    {
        m_world.quit();
    }

    bool Game::run()
    {
        using namespace std::chrono;
        using namespace std::literals;
        using clock = std::chrono::high_resolution_clock;
        static auto loc{ std::locale("en_US.UTF-8") };

        static auto generate_world_entities = [&](uint32_t count) {
            ds::position position{ m_window.center() };

            SetRandomSeed(12345);
            for (size_t i = 0; i < count; ++i)
            {
                Color color{ rand_color(GetRandomValue(0, 64)) };
                std::string name{ fmt::format("Ball {}", i) };
                ds::velocity velocity{
                    static_cast<float>(GetRandomValue(-200, 200)) / 1.0f,
                    static_cast<float>(GetRandomValue(-200, 200)) / 1.0f,
                };

                m_world.entity(name.data())
                    .set<asdf::position>({ position.x, position.y })
                    .set<asdf::velocity>({ velocity.x, velocity.y })
                    .set<asdf::style>({ color });
            }
        };

        auto time_pregen{ clock::now() };

        generate_world_entities(100000);

        auto time_postgen = duration_cast<milliseconds>(clock::now() - time_pregen);
        fmt::print("\nEntity creation time: [{:L}ms]\n", time_postgen.count());

        auto update_count{ 0 };
        auto last_update{ clock::now() };
        while (!this->should_quit())
        {
            float delta_time{ this->delta_time() };
            using us_tm = nanoseconds;
            auto mdelta = clock::now() - last_update;
            last_update = clock::now();
            auto ddelta = to_durations<milliseconds, nanoseconds, microseconds>(mdelta);
            auto ecount = m_world.count<asdf::velocity>();

            if (++update_count % 60 == 0)
            {
                fmt::print(
                    "delta_time:[{:<6L}s]  highres_clock:[ {:>6}ms | {:>6}us | {:>6}ns] ]  updates:[{:L}] entities [{:L}]\n",
                    delta_time, std::get<0>(ddelta).count(), std::get<1>(ddelta).count(),
                    std::get<2>(ddelta).count(), update_count, ecount);
            }

            this->update(delta_time);
            this->render(delta_time);

            m_world.progress(delta_time);
        }

        return 0;
    }

    constexpr int rect_radius = 10;

    // clang-format off
    void Game::update(float delta_time)
    {
        static auto const dims{ m_window.render_size() };

        m_world.each(
            [&](flecs::entity, asdf::position& p, asdf::velocity& v, asdf::style&) {
                p.x += v.x * delta_time;
                p.y += v.y * delta_time;

                bool top_collision = p.y <= rect_radius;
                bool left_collision = p.x <= rect_radius;
                bool right_collision = p.x >= (dims.width - rect_radius);
                bool bottom_collision = p.y >= (dims.height - rect_radius);

                if (right_collision || left_collision)
                    v.x *= -1.0f;
                if (bottom_collision || top_collision)
                    v.y *= -1.0f;
            });

        m_world.progress(delta_time);
    }

    // clang-format on

    void Game::render(float)
    {
        m_window.render([&]() {
            ClearBackground(color::lightgray);

            m_world.each([](const asdf::position& p, const asdf::velocity&, const asdf::style& s) {
                DrawRectangle(static_cast<int>(p.x) - rect_radius,
                              static_cast<int>(p.y) - rect_radius, static_cast<int>(rect_radius * 2U),
                              static_cast<int>(rect_radius * 2U), s.color);
            });

            DrawFPS(10, 10);
        });
    }

    bool Game::should_quit()
    {
        return m_world.should_quit() || m_window.should_close();
    }
}
