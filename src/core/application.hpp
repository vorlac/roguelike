#pragma once

#include "core/display.hpp"
#include "core/window.hpp"
#include "utils/scoped_render.hpp"

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
        Display m_display{};
        uint32_t m_fps_target{ 120 };
    };
}
