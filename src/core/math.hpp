#pragma once

namespace rl::math
{
    inline float inverse_lerp(float from, float to, float val)
    {
        return (val - from) / (to - from);
    }
}
