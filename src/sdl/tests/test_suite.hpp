#pragma once

namespace rl::sdl::test {
    class Window;
    class Renderer;

    int execute_render_tests(sdl::Window& main_window);
    int execute_sprite_drawing_tests(sdl::Window& window);
}
