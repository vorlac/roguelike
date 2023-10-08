#pragma once

#include "math/dimension2d.hpp"
#include "math/point2d.hpp"
#include "math/vector2d.hpp"

#include <raylib.h>
#include <string>
#include <utility>
#include <vector>

namespace rl
{
    class Window
    {
    public:
        inline Window()
        {
            this->init();
        }

        inline Window(const dims2i& dimensions, const std::string& title = "")
            : m_dims{ dimensions }
            , m_title{ title }
        {
            this->init(m_dims.width, m_dims.height, m_title);
        }

        inline Window(Window&& window)
        {
            *this = std::move(window);
            this->init(m_dims.width, m_dims.height, m_title);
        }

        inline ~Window()
        {
            this->teardown();
        }

        inline dims2i screen_dims() const
        {
            return m_dims;
        }

        /**
         * @brief  Check if window has been initialized successfully
         * @return bool
         */
        inline bool is_ready() const
        {
            return IsWindowReady();
        }

        /**
         * @brief Check if KEY_ESCAPE pressed or Close icon pressed
         * @return bool
         */
        inline bool should_close() const
        {
            return WindowShouldClose();
        }

        /**
         * @brief Check if window is currently fullscreen
         * @return bool
         */
        inline bool is_fullscreen() const
        {
            return IsWindowFullscreen();
        }

        /**
         * @brief Check if window is currently hidden
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_hidden() const
        {
            RLAPI bool IsWindowHidden(void);
        }

        /**
         * @brief Check if window is currently minimized
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_minimized() const
        {
            RLAPI bool IsWindowMinimized();
        }

        /**
         * @brief Check if window is currently maximized
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_maximized() const
        {
            return IsWindowMaximized();
        }

        /**
         * @brief Check if window is currently focused
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_focused() const
        {
            return IsWindowFocused();
        }

        /**
         * @brief Check if window has been resized last frame
         * @return bool
         */
        inline bool is_resized() const
        {
            return IsWindowResized();
        }

        /**
         * @brief Check if one specific window flag is enabled
         * @param flag the single state flag to check
         * @return bool
         */
        inline bool is_state(uint32_t flag) const
        {
            return IsWindowState(flag);
        }

        /**
         * @brief Set window configuration state using flags
         * @note PLATFORM_DESKTOP only
         * @param flag the state flags to set
         * @return bool
         */
        inline void set_state(uint32_t flags) const
        {
            return SetWindowState(flags);
        }

        /**
         * @brief Clear window configuration state flags
         * @return void
         */
        inline void clear_state(uint32_t flags) const
        {
            return ClearWindowState(flags);
        }

        /**
         * @brief Toggle window state: fullscreen/windowed
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void toggle_fullscreen() const
        {
            return ToggleFullscreen();
        }

        /**
         * @brief Set window state: maximized, if resizable
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void maximize() const
        {
            return MaximizeWindow();
        }

        /**
         * @brief Set window state: minimized, if resizable
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void minimize() const
        {
            return MinimizeWindow();
        }

        /**
         * @brief Set window state: not minimized/maximized
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void restore() const
        {
            return RestoreWindow();
        }

        /**
         * @brief Set icon for window
         * @note single image, RGBA 32bit, only PLATFORM_DESKTOP
         * @return void
         */
        inline void set_icon(Image&& image) const
        {
            return SetWindowIcon(image);
        }

        /**
         * @brief Set icon for window
         * @note multiple images, RGBA 32bit, only PLATFORM_DESKTOP
         * @return void
         */
        inline void set_icons(std::vector<Image>&& images) const
        {
            return SetWindowIcons(images.data(), static_cast<int>(images.size()));
        }

        /**
         * @brief Set title for window
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void title(std::string&& title) const
        {
            return SetWindowTitle(title.c_str());
        }

        /**
         * @brief Set window position on screen
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void xxxxx(point2i&& pos) const
        {
            return SetWindowPosition(pos.x, pos.y);
        }

        /**
         * @brief Set monitor for the current window (fullscreen mode)
         * @return void
         */
        inline void xxxxx(uint16_t monitor) const
        {
            return SetWindowMonitor(monitor);
        }

        /**
         * @brief Set window minimum dimensions (for FLAG_WINDOW_RESIZABLE)
         * @return void
         */
        inline void min_size(dims2i&& min_size) const
        {
            return SetWindowMinSize(min_size.width, min_size.height);
        }

        /**
         * @brief Set window dimensions
         * @return void
         */
        inline void size(dims2i&& size) const
        {
            return SetWindowSize(size.width, size.height);
        }

        /**
         * @brief Set window opacity [0.0f..1.0f] (only PLATFORM_DESKTOP)
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void opacity(float opacity) const
        {
            return SetWindowOpacity(opacity);
        }

        /**
         * @brief Get native window handle
         * @return void*
         */
        inline void* handle() const
        {
            return GetWindowHandle();
        }

        /**
         * @brief Get current screen dimensions
         * @return void
         */
        inline dims2i screen_size() const
        {
            return {
                .width = GetScreenWidth(),
                .height = GetScreenHeight(),
            };
        }

        /**
         * @brief Get current render dimensions
         * @note it considers HiDPI
         * @return void
         */
        inline dims2i render_size() const
        {
            return {
                .width = GetRenderWidth(),
                .height = GetRenderHeight(),
            };
        }

        /**
         * @brief Get window position XY on monitor
         * @return void
         */
        inline point2f position() const
        {
            return GetWindowPosition();
        }

        /**
         * @brief Get window scale DPI factor
         * @return void
         */
        vector2f scale_dpi_factor() const
        {
            return GetWindowScaleDPI();
        }

    public:
        Window& operator=(Window& window) = delete;
        Window& operator=(const Window& window) = delete;

        Window& operator=(Window&& window)
        {
            m_dims = std::move(window.m_dims);
            m_title = std::move(window.m_title);
            return *this;
        }

    private:
        inline bool init(int32_t width = 1024, int32_t height = 768, const std::string& title = "") const
        {
            InitWindow(width, height, title.c_str());
            return true;
        }

        inline bool teardown() const
        {
            if (IsWindowReady())
                CloseWindow();

            return true;
        }

    private:
        dims2i m_dims{
            .width = 1024,
            .height = 768,
        };

        std::string m_title{ "roguelike" };
    };

}
