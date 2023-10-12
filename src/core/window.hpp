#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <raylib.h>

#include "ds/dimensions.hpp"
#include "ds/point.hpp"
#include "ds/vector2d.hpp"

namespace rl
{
    class Window
    {
    public:
        inline Window()
        {
            this->init();
        }

        inline Window(dimensions<int32_t> dimensions, const std::string& title = "")
        {
            this->init(dimensions.width, dimensions.height, title);
        }

        inline ~Window()
        {
            this->teardown();
        }

        /**
         * @brief Scoped render call that will execute the render logic passed in wrapped in calls
         * @param render_func a callable type (lambda / std::function / functor) including render
         * logic that should be wrapped in BeginDrawing and EndDrawing calls
         * */
        inline void render(auto render_func) const
        {
            this->begin_drawing();
            render_func();
            this->end_drawing();
        }

        /**
         * @brief Setup canvas (framebuffer) to start drawing
         * */
        inline void begin_drawing() const
        {
            return ::BeginDrawing();
        }

        /**
         * @brief End canvas drawing and swap buffers (double buffering)
         * */
        inline void end_drawing() const
        {
            return ::EndDrawing();
        }

        /**
         * @brief  Check if window has been initialized successfully
         * @return bool
         */
        inline bool is_ready() const
        {
            return ::IsWindowReady();
        }

        /**
         * @brief Check if KEY_ESCAPE pressed or Close icon pressed
         * @return bool
         */
        inline bool should_close() const
        {
            return ::WindowShouldClose();
        }

        /**
         * @brief Close window and unload OpenGL context
         */
        inline void close() const
        {
            return ::CloseWindow();
        }

        /**
         * @brief Check if window is currently fullscreen
         * @return bool
         */
        inline bool is_fullscreen() const
        {
            return ::IsWindowFullscreen();
        }

        /**
         * @brief Check if window is currently hidden
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_hidden() const
        {
            return ::IsWindowHidden();
        }

        /**
         * @brief Check if window is currently minimized
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_minimized() const
        {
            return ::IsWindowMinimized();
        }

        /**
         * @brief Check if window is currently maximized
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_maximized() const
        {
            return ::IsWindowMaximized();
        }

        /**
         * @brief Check if window is currently focused
         * @note PLATFORM_DESKTOP only
         * @return bool
         */
        inline bool is_focused() const
        {
            return ::IsWindowFocused();
        }

        /**
         * @brief Check if window has been resized last frame
         * @return bool
         */
        inline bool is_resized() const
        {
            return ::IsWindowResized();
        }

        /**
         * @brief Check if one specific window flag is enabled
         * @param flag the single state flag to check
         * @return bool
         */
        inline bool is_state(uint32_t flag) const
        {
            return ::IsWindowState(flag);
        }

        /**
         * @brief Set window configuration state using flags
         * @note PLATFORM_DESKTOP only
         * @param flag the state flags to set
         * @return bool
         */
        inline void set_state(uint32_t flags) const
        {
            return ::SetWindowState(flags);
        }

        /**
         * @brief Clear window configuration state flags
         * @return void
         */
        inline void clear_state(uint32_t flags) const
        {
            return ::ClearWindowState(flags);
        }

        /**
         * @brief Toggle window state: fullscreen/windowed
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void toggle_fullscreen() const
        {
            return ::ToggleFullscreen();
        }

        /**
         * @brief Set window state: maximized, if resizable
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void maximize() const
        {
            return ::MaximizeWindow();
        }

        /**
         * @brief Set window state: minimized, if resizable
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void minimize() const
        {
            return ::MinimizeWindow();
        }

        /**
         * @brief Set window state: not minimized/maximized
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void restore() const
        {
            return ::RestoreWindow();
        }

        /**
         * @brief Set icon for window
         * @note single image, RGBA 32bit, only PLATFORM_DESKTOP
         * @return void
         */
        inline void set_icon(::Image&& image) const
        {
            return ::SetWindowIcon(image);
        }

        /**
         * @brief Set icon for window
         * @note multiple images, RGBA 32bit, only PLATFORM_DESKTOP
         * @return void
         */
        inline void set_icons(std::vector<::Image>&& images) const
        {
            return ::SetWindowIcons(images.data(), static_cast<int>(images.size()));
        }

        /**
         * @brief Set title for window
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void title(std::string&& title) const
        {
            return ::SetWindowTitle(title.c_str());
        }

        /**
         * @brief Set window position on screen
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void set_position(position<int32_t>&& pos) const
        {
            return ::SetWindowPosition(pos.x, pos.y);
        }

        /**
         * @brief Set monitor for the current window
         * @note fullscreen mode
         * @return void
         */
        inline void set_monitor(uint16_t monitor) const
        {
            return ::SetWindowMonitor(monitor);
        }

        /**
         * @brief Set window minimum dimensions
         * @param min_size The minimum window dimensions to set
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void min_size(dimensions<int32_t> min_size) const
        {
            return ::SetWindowMinSize(min_size.width, min_size.height);
        }

        /**
         * @brief Set window dimensions
         * @param size The window dimensions to set
         * @return void
         */
        inline void size(dimensions<int32_t> size) const
        {
            return ::SetWindowSize(size.width, size.height);
        }

        /**
         * @brief Sets window opacity
         * @param opacity The window opacity to set [0.0f..1.0f]
         * @note PLATFORM_DESKTOP only
         * @return void
         */
        inline void opacity(float opacity) const
        {
            return ::SetWindowOpacity(opacity);
        }

        /**
         * @brief Get native window handle
         * @return void* window handle pointer
         */
        inline void* handle() const
        {
            return ::GetWindowHandle();
        }

        /**
         * @brief Get current screen dimensions
         * @return void
         */
        inline dimensions<int32_t> screen_size() const
        {
            return {
                .width = ::GetScreenWidth(),
                .height = ::GetScreenHeight(),
            };
        }

        /**
         * @brief Get current render dimensions
         * @note it considers HiDPI
         * @return void
         */
        inline dimensions<int32_t> render_size() const
        {
            return {
                .width = ::GetRenderWidth(),
                .height = ::GetRenderHeight(),
            };
        }

        /**
         * @brief Get window position XY on monitor
         * @return void
         */
        inline point<float> position() const
        {
            return ::GetWindowPosition();
        }

        /**
         * @brief Get window scale DPI factor
         * @return void
         */
        vector2<float> scale_dpi_factor() const
        {
            return ::GetWindowScaleDPI();
        }

    public:
        Window& operator=(Window window) = delete;
        Window& operator=(Window& window) = delete;
        Window& operator=(Window&& window) = delete;
        Window& operator=(const Window& window) = delete;

    private:
        inline bool init(int32_t width = 1024, int32_t height = 768, const std::string& title = "") const
        {
            ::InitWindow(width, height, title.c_str());
            return true;
        }

        inline bool teardown() const
        {
            if (this->is_ready())
                this->close();

            return true;
        }
    };
}
