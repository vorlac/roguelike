#pragma once

namespace rl::component
{
    struct velocity
    {
        float x{ 0.0 };
        float y{ 0.0 };
    };

    struct max_speed
    {
        float x{ 10.0 };
        float y{ 10.0 };
    };

    struct min_speed
    {
        float x{ 10.0 };
        float y{ 10.0 };
    };
}
