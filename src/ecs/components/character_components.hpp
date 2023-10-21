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
        int32_t amount{ 100 };
    };

    struct move_speed
    {
        int32_t speed{ 100 };
    };
}
