#include <cstdio>
#include <memory>

#include "core/application.hpp"
#include "core/utils/conversions.hpp"
#include "thirdparty/raylib.hpp"

namespace rl
{
    Application::Application()
    {
        this->setup();
    }

    Application::Application(ds::dimensions<i32> dims, std::string title, u32 fps)
        : m_window(std::forward<ds::dimensions<i32>>(dims), std::forward<std::string>(title))
    {
        this->setup(fps);
    }

    Application::~Application()
    {
        this->teardown();
    }

    u32 Application::framerate()
    {
        return cast::to<u32>(raylib::GetFPS());
    }

    void Application::framerate(u32 target_fps)
    {
        raylib::SetTargetFPS(cast::to<i32>(target_fps));
    }

    f32 Application::delta_time()
    {
        return m_window.frame_time();
    }

    void Application::clipboard_text(std::string text)
    {
        return raylib::SetClipboardText(text.c_str());
    }

    std::string Application::clipboard_text()
    {
        return raylib::GetClipboardText();
    }

    void Application::enable_event_waiting()
    {
        return raylib::EnableEventWaiting();
    }

    void Application::disable_event_waiting()
    {
        return raylib::DisableEventWaiting();
    }

    void Application::setup(u32 fps_target)
    {
        this->framerate(fps_target);
    }

    void Application::teardown()
    {
    }
}
