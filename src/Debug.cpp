#include "Debug.h"

#include <raylib.h>

void DrawChunkDebug()
{
    int WIDTH = 32;
    int HEIGHT = 32;

    int T_WIDTH = 32;
    int T_HEIGHT = 32;

    Vector2 offset{ 10, 10 };

    Color color{ 255, 255, 255, 255 };

    DrawRectangleLines(static_cast<int>(offset.x), static_cast<int>(offset.y), T_WIDTH * WIDTH,
                       T_HEIGHT * HEIGHT, color);

    for (int y = 0; y <= HEIGHT; y++)
    {
        int y_height = static_cast<int>((y * T_HEIGHT) + offset.y);
        DrawLine(static_cast<int>(offset.x), static_cast<int>(y_height),
                 (WIDTH * T_WIDTH) + static_cast<int>(offset.x), static_cast<int>(y_height), color);
    }
    for (int x = 0; x <= WIDTH; x++)
    {
        int x_height = (x * T_WIDTH + static_cast<int>(offset.x));
        DrawLine(x_height, static_cast<int>(offset.y), x_height,
                 (HEIGHT * T_HEIGHT) + static_cast<int>(offset.y), color);
    }
}
