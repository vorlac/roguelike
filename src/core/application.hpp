#pragma once

#include "core/scoped_render.hpp"
#include "core/window.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <raylib.h>
#include <string>

namespace rl
{
    class Application
    {
    public:
        enum State : uint_fast8_t
        {
        };

    public:
        Application()
        {
            this->init();
        }

        Application(dims2i dimensions, std::string title, uint32_t fps = 120)
            : m_window(std::forward<dims2i>(dimensions), std::forward<std::string>(title))
            , m_fps_target{ fps }
        {
            this->init();
        }

        Application(Application&& application)
        {
            *this = std::move(application);
        }

        ~Application()
        {
            this->teardown();
        }

        inline bool run()
        {
            while (!m_window.should_close())
            {
                this->update();
                this->render();
            }

            return 0;
        }

        inline bool render() const
        {
            static std::string text{};
            text = fmt::format("FPS: {}", this->framerate());

            scoped_render([&] {
                ClearBackground(RAYWHITE);
                DrawText(text.data(), 190, 200, 20, GRAY);
            });

            return true;
        }

        inline bool update() const
        {
            return true;
        }

    public:
        Application& operator=(Application&& application)
        {
            m_fps_target = std::move(application.m_fps_target);
            m_window = std::move(application.m_window);
            return *this;
        }

    public:
        /**
         * @brief Get current FPS
         * @return uint32_t
         */
        inline uint32_t framerate() const
        {
            return static_cast<uint32_t>(GetFPS());
        }

        /**
         * @brief Set target FPS (maximum)
         * @param target_fps
         */
        inline void framerate(uint32_t target_fps)
        {
            m_fps_target = target_fps;
            SetTargetFPS(static_cast<int>(target_fps));
        }

        /**
         * @brief Get time in seconds for last frame drawn
         * @return float
         */
        inline float delta_time() const
        {
            return GetFrameTime();
        }

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
         * @brief  Get specified monitor position
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

        /**
         * @brief Set clipboard text content
         * @param text the text being assigned
         */
        inline void clipboard_text(std::string&& text) const
        {
            return SetClipboardText(text.c_str());
        }

        /**
         * @brief Get clipboard text content
         * @return std::string
         */
        inline std::string clipboard_text() const
        {
            return GetClipboardText();
        }

        /**
         * @brief Enable waiting for events on EndDrawing(),
         * no automatic event polling
         */
        inline void enable_event_waiting() const
        {
            return EnableEventWaiting();
        }

        /**
         * @brief Disable waiting for events on EndDrawing(),
         * automatic events polling
         */
        inline void disable_event_waiting() const
        {
            return DisableEventWaiting();
        }

    private:
        inline bool init()
        {
            this->framerate(m_fps_target);
            return true;
        }

        inline bool teardown()
        {
            // handle cleanup before deinit
            return true;
        }

    private:
        Window m_window{};
        uint32_t m_fps_target{ 120 };
    };
}
