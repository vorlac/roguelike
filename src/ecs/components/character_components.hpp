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
}
