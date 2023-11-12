#pragma once

namespace rl::sdl::test {
    class window;
    class renderer;

    int execute_render_tests(sdl::window& main_window);
    int execute_sprite_drawing_tests(sdl::window& window);
}
