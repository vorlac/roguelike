#include "core/display.hpp"

#include <cstdint>
#include <string>
#include <raylib.h>

namespace rl
{
    int Display::monitor_count()
    {
        return ::GetMonitorCount();
    }

    int32_t Display::current_monitor()
    {
        return ::GetCurrentMonitor();
    }

    ds::position<float> Display::monitor_position(uint16_t monitor)
    {
        // TODO: change to pointi?
        auto pos{ ::GetMonitorPosition(monitor) };
        return ds::position<float>(pos.x, pos.y);
    }

    ds::dimensions<int32_t> Display::monitor_dims(int16_t monitor)
    {
        return {
            .width = ::GetMonitorWidth(monitor),
            .height = ::GetMonitorHeight(monitor),
        };
    }

    ds::dimensions<int32_t> Display::monitor_physical_dims(int16_t monitor)
    {
        return {
            .width = ::GetMonitorPhysicalWidth(monitor),
            .height = ::GetMonitorPhysicalHeight(monitor),
        };
    }

    uint32_t Display::monitor_refresh_rate(uint16_t monitor)
    {
        return ::GetMonitorRefreshRate(monitor);
    }

    std::string Display::monitor_name(uint16_t monitor)
    {
        return ::GetMonitorName(monitor);
    }
}
