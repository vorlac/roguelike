#include <cstdint>
#include <string>

#include "core/display.hpp"
#include "core/numerics.hpp"

#include "thirdparty/raylib.hpp"

namespace rl
{
    i32 Display::monitor_count()
    {
        return raylib::GetMonitorCount();
    }

    i32 Display::current_monitor()
    {
        return raylib::GetCurrentMonitor();
    }

    u32 Display::monitor_refresh_rate(u16 monitor)
    {
        return static_cast<u32>(raylib::GetMonitorRefreshRate(monitor));
    }

    std::string Display::monitor_name(u16 monitor)
    {
        return raylib::GetMonitorName(monitor);
    }

    ds::position<f32> Display::monitor_position(u16 monitor)
    {
        // TODO: change to pointi?
        auto pos{ raylib::GetMonitorPosition(monitor) };
        return ds::position<f32>(pos.x, pos.y);
    }

    ds::dimensions<i32> Display::monitor_dims(i16 monitor)
    {
        return {
            raylib::GetMonitorWidth(monitor),
            raylib::GetMonitorHeight(monitor),
        };
    }

    ds::dimensions<i32> Display::monitor_physical_dims(i16 monitor)
    {
        return {
            raylib::GetMonitorPhysicalWidth(monitor),
            raylib::GetMonitorPhysicalHeight(monitor),
        };
    }
}
