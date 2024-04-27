#pragma once

#include "gfx/vg/nanovg.hpp"

namespace rl::nvg::gl {
    // Create flags

    enum class CreateFlags {
        None = 0,
        // Flag indicating if geometry based anti-aliasing
        // is used (may not be needed when using MSAA).
        AntiAlias = 1 << 0,
        // Flag indicating if strokes should be drawn using stencil buffer. The rendering will be a
        // little slower, but path overlaps (i.e. self-intersecting or sharp turns) will be drawn
        // just once.
        StencilStrokes = 1 << 1,
        // Flag indicating that additional debug checks are done.
        Debug = 1 << 2,
    };

    // These are additional flags on top of nvg::ImageFlags.
    enum class GLImageFlags {
        ImageNoDelete = 1 << 16,  // Do not delete GL texture handle.
    };

    Context* create_gl_context(CreateFlags flags);
    void delete_gl_context(Context* ctx);

    int create_image_from_handle(Context* ctx, GLuint texture_id, int w, int h, int flags);
    GLuint image_handle(Context* ctx, int image);

}
