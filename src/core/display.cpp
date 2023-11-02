#include <cstdint>
#include <string>

#include "core/display.hpp"
#include "core/numeric_types.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    i32 Display::monitor_count() const
    {
        return raylib::GetMonitorCount();
    }

    i32 Display::current_monitor() const
    {
        return raylib::GetCurrentMonitor();
    }

    u32 Display::monitor_refresh_rate(i32 monitor) const
    {
        return cast::to<u32>(raylib::GetMonitorRefreshRate(monitor));
    }

    std::string Display::monitor_name(i32 monitor) const
    {
        return raylib::GetMonitorName(monitor);
    }

    ds::point<f32> Display::monitor_position(i32 monitor) const
    {
        // TODO: change to pointi?
        auto pos{ raylib::GetMonitorPosition(monitor) };
        return ds::point<f32>(pos.x, pos.y);
    }

    ds::dimensions<i32> Display::monitor_dims(i32 monitor) const
    {
        return {
            raylib::GetMonitorWidth(monitor),
            raylib::GetMonitorHeight(monitor),
        };
    }

    ds::dimensions<i32> Display::monitor_physical_dims(i32 monitor) const
    {
        return {
            raylib::GetMonitorPhysicalWidth(monitor),
            raylib::GetMonitorPhysicalHeight(monitor),
        };
    }
}
