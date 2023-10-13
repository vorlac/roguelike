#pragma once

#include "core/application.hpp"

#include <cstdio>
#include <memory>
#include <raylib.h>

namespace rl
{
    Application::Application()
    {
        this->init();
    }

    Application::Application(ds::dimensions<int32_t> dims, std::string title, uint32_t fps)
        : m_window(std::forward<ds::dimensions<int32_t>>(dims), std::forward<std::string>(title))
    {
        this->init(fps);
    }

    Application::~Application()
    {
        this->teardown();
    }

    uint32_t Application::framerate()
    {
        return static_cast<uint32_t>(::GetFPS());
    }

    void Application::framerate(uint32_t target_fps)
    {
        ::SetTargetFPS(static_cast<int>(target_fps));
    }

    float Application::delta_time()
    {
        return ::GetFrameTime();
    }

    void Application::clipboard_text(std::string text)
    {
        return ::SetClipboardText(text.c_str());
    }

    std::string Application::clipboard_text()
    {
        return ::GetClipboardText();
    }

    void Application::enable_event_waiting()
    {
        return ::EnableEventWaiting();
    }

    void Application::disable_event_waiting()
    {
        return ::DisableEventWaiting();
    }

    bool Application::init(uint32_t fps_target)
    {
        this->framerate(fps_target);
        return true;
    }

    bool Application::teardown()
    {
        return true;
    }

}
