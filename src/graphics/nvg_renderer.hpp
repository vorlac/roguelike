#pragma once

#include "core/assert.hpp"
#include "core/ui/theme.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"

namespace rl::nvg {
    struct NVGcontext;
}

namespace rl {
    class VectorizedRenderer
    {
    public:
        VectorizedRenderer();

        nvg::NVGcontext* nvg_context() const;

        void flush(ds::dims<i32> viewport, f32 pixel_ratio);

        void begin_frame();
        void end_frame();

        // saves the current context render state to an internal
        // stack that tracks all theme, text, AA, etc settings
        void push_render_state();

        // pops the current context render state off of the internal
        // stack to restore all theme, text, AA, etc settings
        void pop_render_state();

        // Draws rectangle outline in a specific width, color,
        // and location (inner vs outer outline)
        void draw_rect_outline(ds::rect<f32> rect, f32 stroke_width, ds::color<f32> color,
                               ui::Outline type);

    private:
        bool m_depth_buffer{ false };
        bool m_stencil_buffer{ false };
        bool m_float_buffer{ false };
        nvg::NVGcontext* m_nvg_context{ nullptr };
    };
}
