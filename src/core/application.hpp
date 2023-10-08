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
        Application(dims2i dimensions = { .width = 1920, .height = 1080 }, std::string title = "")
            : m_window(dimensions, title)
        {
            // this->fps(60);
        }

        bool run()
        {
            while (!m_window.should_close())
            {
                this->update();
                this->render();
            }

            return 0;
        }

        bool render()
        {
            const std::string text{ fmt::format("FPS: {}", this->fps()) };

            scoped_render([&] {
                ClearBackground(RAYWHITE);
                DrawText(text.data(), 190, 200, 20, GRAY);
            });

            return true;
        }

        bool update()
        {
            return true;
        }

        uint32_t fps()
        {
            return static_cast<uint32_t>(GetFPS());
        }

        void fps(uint32_t target_fps)
        {
            SetTargetFPS(static_cast<int>(target_fps));
        }

        float frametime()
        {
            return GetFrameTime();
        }

    private:
        Window m_window{};
    };
}
