#pragma once

#include "graphics/vg/nanovg.hpp"

namespace rl::nvg {
    // Create flags

    enum NVGcreateFlags {
        // Flag indicating if geometry based anti-aliasing is used (may not be needed when using
        // MSAA).
        NVG_ANTIALIAS = 1 << 0,
        // Flag indicating if strokes should be drawn using stencil buffer. The rendering will be a
        // little slower, but path overlaps (i.e. self-intersecting or sharp turns) will be drawn
        // just once.
        NVG_STENCIL_STROKES = 1 << 1,
        // Flag indicating that additional debug checks are done.
        NVG_DEBUG = 1 << 2,
    };

    // Define VTable with pointers to the functions for a each OpenGL (ES) version.

    using NanoVG_GL_Functions_VTable = struct
    {
        const char* name;
        NVGcontext* (*createContext)(int flags);
        void (*deleteContext)(NVGcontext* ctx);
        int (*createImageFromHandle)(NVGcontext* ctx, unsigned int textureId, int w, int h,
                                     int flags);
        unsigned int (*getImageHandle)(NVGcontext* ctx, int image);
    };

    // Create NanoVG contexts for different OpenGL (ES) versions.

    NVGcontext* nvgCreateGL2(int flags);
    void nvgDeleteGL2(NVGcontext* ctx);

    int nvglCreateImageFromHandleGL2(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
    GLuint nvglImageHandleGL2(NVGcontext* ctx, int image);

    NVGcontext* nvg_create_gl3(int flags);
    void nvg_delete_gl3(NVGcontext* ctx);

    int nvgl_create_image_from_handle_gl3(NVGcontext* ctx, GLuint textureId, int w, int h,
                                          int flags);
    GLuint nvgl_image_handle_gl3(NVGcontext* ctx, int image);

    NVGcontext* nvgCreateGLES2(int flags);
    void nvgDeleteGLES2(NVGcontext* ctx);

    int nvglCreateImageFromHandleGLES2(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
    GLuint nvglImageHandleGLES2(NVGcontext* ctx, int image);

    NVGcontext* nvgCreateGLES3(int flags);
    void nvgDeleteGLES3(NVGcontext* ctx);

    int nvglCreateImageFromHandleGLES3(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
    GLuint nvglImageHandleGLES3(NVGcontext* ctx, int image);

    // These are additional flags on top of NVGimageFlags.
    enum NVGimageFlagsGL {
        NVG_IMAGE_NODELETE = 1 << 16,
        // Do not delete GL texture handle.
    };

    // Create VTables for different OpenGL (ES) versions.

    extern const NanoVG_GL_Functions_VTable NanoVG_GL2_Functions_VTable;
    extern const NanoVG_GL_Functions_VTable NANO_VG_GL3_FUNCTIONS_V_TABLE;
    extern const NanoVG_GL_Functions_VTable NanoVG_GLES2_Functions_VTable;
    extern const NanoVG_GL_Functions_VTable NanoVG_GLES3_Functions_VTable;
}
