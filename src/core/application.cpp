#pragma once

#include "core/application.hpp"

#include <cstdio>
#include <memory>

#include "thirdparty/raylib.hpp"

namespace rl
{
    Application::Application()
    {
        this->setup();
    }

    Application::Application(ds::dimensions<int32_t> dims, std::string title, uint32_t fps)
        : m_window(std::forward<ds::dimensions<int32_t>>(dims), std::forward<std::string>(title))
    {
        this->setup(fps);
    }

    Application::~Application()
    {
        this->teardown();
    }

    uint32_t Application::framerate()
    {
        return static_cast<uint32_t>(raylib::GetFPS());
    }

    void Application::framerate(uint32_t target_fps)
    {
        raylib::SetTargetFPS(static_cast<int>(target_fps));
    }

    float Application::delta_time()
    {
        return raylib::GetFrameTime();
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

    void Application::setup(uint32_t fps_target)
    {
        this->framerate(fps_target);
    }

    void Application::teardown()
    {
    }

}
