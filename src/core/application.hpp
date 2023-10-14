#pragma once

#include <string>
#include <raylib.h>

#include "core/display.hpp"
#include "core/window.hpp"
#include "ds/dimensions.hpp"

namespace rl
{
    class Application
    {
        static constexpr inline auto DefaultFPS{ 1000 };

    public:
        Application();
        Application(ds::dimensions<int32_t> dims, std::string title, uint32_t fps = DefaultFPS);
        ~Application();

    public:
        uint32_t framerate();
        void framerate(uint32_t target_fps);
        float delta_time();
        void clipboard_text(std::string text);
        std::string clipboard_text();
        void enable_event_waiting();
        void disable_event_waiting();

    protected:
        void init(uint32_t fps_target = DefaultFPS);
        void teardown();

    protected:
        rl::Window m_window{};
        rl::Display m_display{};
    };
}
