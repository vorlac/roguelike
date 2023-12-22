#pragma once

#include <glad/gl.h>

#include <nanovg.h>

#pragma warning(disable : 4244)

namespace rl::ui {

    inline bool nvg_is_image_icon(int value)
    {
        return value < 1024;
    }

    inline bool nvg_is_font_icon(int value)
    {
        return value >= 1024;
    }

    extern bool nanogui_check_glerror(const char* cmd);
}
