#pragma once

#include <string>

#include "core/display.hpp"
#include "core/ds/dimensions.hpp"
#include "core/input/input.hpp"
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
        u32 framerate();
        void framerate(u32 target_fps);
        float delta_time();
        void clipboard_text(std::string text);
        std::string clipboard_text();
        void enable_event_waiting();
        void disable_event_waiting();

    protected:
        void setup(u32 fps_target = DefaultFPS);
        void teardown();

    protected:
        rl::Window m_window{};
        rl::Display m_display{};
        rl::input::Input m_input{};
    };
}
