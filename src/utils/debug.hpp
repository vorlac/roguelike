#pragma once

#include "ds/color.hpp"

namespace rl::debug {
    struct ui
    {
        constexpr static bool mouse_interaction{ true };
        constexpr static bool widget_outlines{ true };

        constexpr static ds::color<f32> layout_background_color{ 40, 44, 52 };
        constexpr static ds::color<f32> widget_outline_color{ Colors::Blue };
        constexpr static ds::color<f32> active_outline_color{ Colors::Yellow };
    };

    struct core
    {
        constexpr static bool loop_timing_stats{ true };
        constexpr static bool loop_throttling{ true };
    };
}
