#pragma once

#include <concepts>
#include <functional>
#include <raylib.h>
#include <type_traits>

namespace rl
{
    inline void scoped_render(std::function<void()>&& render_func)
    {
        BeginDrawing();
        render_func();
        EndDrawing();
    }
}
