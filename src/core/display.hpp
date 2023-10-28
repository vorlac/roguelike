#pragma once

#include <cstdint>
#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/numerics.hpp"

namespace rl
{
    class Display
    {
    public:
        i32 monitor_count();
        i32 current_monitor();
        u32 monitor_refresh_rate(u16 monitor);
        std::string monitor_name(u16 monitor);
        ds::position<f32> monitor_position(u16 monitor);
        ds::dimensions<i32> monitor_dims(i16 monitor);
        ds::dimensions<i32> monitor_physical_dims(i16 monitor);
    };
}
