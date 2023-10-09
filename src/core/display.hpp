#include <cstdint>
#include <string>
#include <raylib.h>

#include "math/dimension2d.hpp"
#include "math/point2d.hpp"

namespace rl
{
    class Display
    {
    public:
        Display() = default;

        /**
         * @brief Get number of connected monitors
         * @return int
         */
        inline int monitor_count() const
        {
            return GetMonitorCount();
        }

        /**
         * @brief Get current connected monitor
         * @return int32_t
         */
        inline int32_t current_monitor() const
        {
            return GetCurrentMonitor();
        }

        /**
         * @brief Get specified monitor position
         * @param monitor
         * @return point2f&&
         */
        inline point2f&& monitor_position(uint16_t monitor) const
        {
            // TODO: change to point2i?
            return GetMonitorPosition(monitor);
        }

        /**
         * @brief Get specified monitor width (current video mode used by monitor)
         * @param monitor
         * @return dims2i&&
         */
        inline dims2i&& monitor_dims(uint16_t monitor) const
        {
            return {
                .width = GetMonitorWidth(monitor),
                .height = GetMonitorHeight(monitor),
            };
        }

        /**
         * @brief Get specified monitor physical dimensions in millimetres
         * @param monitor
         * @return dims2i&&
         */
        inline dims2i&& monitor_physical_dims(uint16_t monitor) const
        {
            return {
                .width = GetMonitorPhysicalWidth(monitor),
                .height = GetMonitorPhysicalHeight(monitor),
            };
        }

        /**
         * @brief Get specified monitor refresh rate
         * @param monitor
         * @return uint32_t
         */
        inline uint32_t monitor_refresh_rate(uint16_t monitor) const
        {
            return GetMonitorRefreshRate(monitor);
        }

        /**
         * @brief Get the human-readable, UTF-8 encoded name of the primary monitor
         * @param monitor
         * @return std::string
         */
        inline std::string monitor_name(uint16_t monitor) const
        {
            return GetMonitorName(monitor);
        }
    };
}
