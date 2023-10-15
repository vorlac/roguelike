#pragma once

#include <flecs.h>

#include "core/application.hpp"
#include "ds/dimensions.hpp"

namespace rl
{
    class Game : public Application
    {
    public:
        bool run();
        bool init();
        bool teardown();

        void update(float delta_time);
        void render(float delta_time);

        bool should_quit() const;
        void quit() const;

    protected:
        flecs::world m_world{};

    private:
        inline static constexpr ds::dimensions rect_size{
            .width = 10,
            .height = 10,
        };
    };
}
