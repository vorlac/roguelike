#include <cstdint>
#include <string>

#include "core/display.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    int32_t Display::monitor_count()
    {
        return raylib::GetMonitorCount();
    }

    int32_t Display::current_monitor()
    {
        return raylib::GetCurrentMonitor();
    }

    uint32_t Display::monitor_refresh_rate(uint16_t monitor)
    {
        return static_cast<uint32_t>(raylib::GetMonitorRefreshRate(monitor));
    }

    std::string Display::monitor_name(uint16_t monitor)
    {
        return raylib::GetMonitorName(monitor);
    }

    ds::position<float> Display::monitor_position(uint16_t monitor)
    {
        // TODO: change to pointi?
        auto pos{ raylib::GetMonitorPosition(monitor) };
        return ds::position<float>(pos.x, pos.y);
    }

    ds::dimensions<int32_t> Display::monitor_dims(int16_t monitor)
    {
        return {
            .width  = raylib::GetMonitorWidth(monitor),
            .height = raylib::GetMonitorHeight(monitor),
        };
    }

    ds::dimensions<int32_t> Display::monitor_physical_dims(int16_t monitor)
    {
        return {
            .width  = raylib::GetMonitorPhysicalWidth(monitor),
            .height = raylib::GetMonitorPhysicalHeight(monitor),
        };
    }
}
