#pragma once

namespace rl::component
{
    struct position
    {
        float x{ 0.0 };
        float y{ 0.0 };
    };

    struct rotation
    {
        float angle{ 0.0 };
    };

    struct scale
    {
        float factor{ 0.0 };
    };
}
