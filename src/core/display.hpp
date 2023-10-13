#pragma once

#include <cstdint>
#include <string>

#include "ds/dimensions.hpp"
#include "ds/point.hpp"

namespace rl
{
    class Display
    {
    public:
        int monitor_count();
        int32_t current_monitor();
        ds::position<float> monitor_position(uint16_t monitor);
        ds::dimensions<int32_t> monitor_dims(int16_t monitor);
        ds::dimensions<int32_t> monitor_physical_dims(int16_t monitor);
        uint32_t monitor_refresh_rate(uint16_t monitor);
        std::string monitor_name(uint16_t monitor);
    };
}
