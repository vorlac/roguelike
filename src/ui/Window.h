#pragma once

#include "Settings.h"

#include <raylib.h>
#include <string>

struct Window
{
    Rectangle rect;
    std::string title;
    bool centered;

    Window(int x, int y, int w, int h, std::string _title)
    {
        rect.x = static_cast<float>(x);
        rect.y = static_cast<float>(y);
        rect.width = static_cast<float>(w);
        rect.height = float(h);
        title = title;
        centered = false;
    }
};

void DrawWindow(int x, int y, int w, int h, std::string title, Color bgC, Color tbC, Color bbC);
void DrawWindow(Window window, Settings settings);
