#pragma once

#include <string>

#include "core/display.hpp"
#include "core/ds/dimensions.hpp"
#include "core/input.hpp"
#include "core/numeric_types.hpp"
#include "core/window.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    class Application
    {
        static constexpr inline auto DefaultFPS{ 1000 };

    public:
        Application();
        Application(ds::dimensions<i32> dims, std::string title, u32 fps = DefaultFPS);
        ~Application();

    public:
        f32 delta_time();
        u32 framerate();
        void framerate(u32 target_fps);
        void enable_event_waiting();
        void disable_event_waiting();
        void clipboard_text(std::string text);
        std::string clipboard_text();

    protected:
        void setup(u32 fps_target = DefaultFPS);
        void teardown();

    protected:
        rl::Window m_window{};
    };
}
