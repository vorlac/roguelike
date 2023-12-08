#pragma once

namespace rl::sdl::test {
    class Window;

    int execute_render_tests(std::unique_ptr<Window>& main_window);
    int execute_sprite_drawing_tests(std::unique_ptr<Window>& window);
}
