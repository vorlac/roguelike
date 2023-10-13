#pragma once

#include <string>
#include <fmt/core.h>
#include <fmt/format.h>
#include <raylib.h>

#include "core/display.hpp"
#include "core/window.hpp"

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

        Application(ds::dimensions<int32_t> dims, std::string title, uint32_t fps = 120)
            : m_window(std::forward<ds::dimensions<int32_t>>(dims), std::forward<std::string>(title))
        {
            this->init(fps);
        }

        ~Application()
        {
            this->teardown();
        }

    public:
        /**
         * @brief Get current FPS
         * @return uint32_t
         */
        inline uint32_t framerate() const
        {
            return static_cast<uint32_t>(::GetFPS());
        }

        /**
         * @brief Set target FPS (maximum)
         * @param target_fps
         */
        inline void framerate(uint32_t target_fps)
        {
            ::SetTargetFPS(static_cast<int>(target_fps));
        }

        /**
         * @brief Get time in seconds for last frame drawn
         * @return float
         */
        inline float delta_time() const
        {
            return ::GetFrameTime();
        }

        /**
         * @brief Set clipboard text content
         * @param text the text being assigned
         */
        inline void clipboard_text(std::string&& text) const
        {
            return ::SetClipboardText(text.c_str());
        }

        /**
         * @brief Get clipboard text content
         * @return std::string
         */
        inline std::string clipboard_text() const
        {
            return ::GetClipboardText();
        }

        /**
         * @brief Enable waiting for events on EndDrawing(),
         * no automatic event polling
         */
        inline void enable_event_waiting() const
        {
            return ::EnableEventWaiting();
        }

        /**
         * @brief Disable waiting for events on EndDrawing(),
         * automatic events polling
         */
        inline void disable_event_waiting() const
        {
            return ::DisableEventWaiting();
        }

    protected:
        inline bool init(uint32_t fps_target = 120)
        {
            this->framerate(fps_target);
            return true;
        }

        inline bool teardown()
        {
            // handle cleanup before deinit
            return true;
        }

    protected:
        Window m_window{};
        Display m_display{};
    };
}
