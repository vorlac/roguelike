#pragma once

#include <cstdint>

namespace rl::component
{
    struct character
    {
        bool alive{ true };
    };

    struct health
    {
        i32 amount{ 100 };
    };

    struct move_speed
    {
        i32 speed{ 100 };
    };
}
