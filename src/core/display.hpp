#pragma once

#include <cstdint>
#include <string>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/numeric_types.hpp"

namespace rl
{
    class Display
    {
    public:
        i32 monitor_count() const;
        i32 current_monitor() const;
        u32 monitor_refresh_rate(i32 monitor) const;
        std::string monitor_name(i32 monitor) const;
        ds::point<f32> monitor_position(i32 monitor) const;
        ds::dimensions<i32> monitor_dims(i32 monitor) const;
        ds::dimensions<i32> monitor_physical_dims(i32 monitor) const;
    };
}
