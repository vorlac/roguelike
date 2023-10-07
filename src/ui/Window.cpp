#include "ui/Window.h"

#include <raylib.h>

void DrawWindow(int x, int y, int w, int h, std::string title, Color bgC, Color tbC, Color bbC)
{
    Vector2 topLeft{ static_cast<float>(x), static_cast<float>(y) };
    Vector2 topRight{ static_cast<float>(x + w), static_cast<float>(y) };
    Vector2 bottomLeft{ static_cast<float>(x), static_cast<float>(y + h) };
    Vector2 bottomRight{ static_cast<float>(x + w), static_cast<float>(y + h) };

    // bg
    DrawRectangle(x, y, w, h, bgC);

    // highlight
    DrawRectangleLines(x, y, w, h, tbC);

    // bottom
    DrawLineEx(bottomLeft, bottomRight, 2, bbC);

    // right
    DrawLineEx(topRight, bottomRight, 2, bbC);
}

void DrawWindow(Window window, Settings settings)
{
    Color topBorderColor = { 100, 109, 110, 255 };
    Color bgColor = { 78, 87, 87, 255 };
    Color bottomBorderColor = { 58, 67, 67, 255 };

    int x = static_cast<int>(window.rect.x);
    int y = static_cast<int>(window.rect.y);

    if (window.centered)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        x = static_cast<int>((sw / 2) - (window.rect.width / 2));
        y = static_cast<int>((sh / 2) - (window.rect.height / 2));
    }
    // todo handle centered
    DrawWindow(x, y, static_cast<int>(window.rect.width), static_cast<int>(window.rect.height),
               window.title, bgColor, topBorderColor, bottomBorderColor);
}
