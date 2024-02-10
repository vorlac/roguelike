#pragma once

#include "graphics/vg/nanovg_gl.hpp"

namespace rl::nvg {
    struct NVGLUframebuffer
    {
        NVGcontext* ctx;
        GLuint fbo;
        GLuint rbo;
        GLuint texture;
        int image;
    };

    // Helper function to create GL frame buffers for different OpenGL (ES) versions.

    void nvgluBindFramebufferGL2(NVGLUframebuffer* fb);
    NVGLUframebuffer* nvgluCreateFramebufferGL2(NVGcontext* ctx, int w, int h, int imageFlags);
    void nvgluDeleteFramebufferGL2(NVGLUframebuffer* fb);

    void nvgluBindFramebufferGL3(NVGLUframebuffer* fb);
    NVGLUframebuffer* nvgluCreateFramebufferGL3(NVGcontext* ctx, int w, int h, int imageFlags);
    void nvgluDeleteFramebufferGL3(NVGLUframebuffer* fb);

    void nvgluBindFramebufferGLES2(NVGLUframebuffer* fb);
    NVGLUframebuffer* nvgluCreateFramebufferGLES2(NVGcontext* ctx, int w, int h, int imageFlags);
    void nvgluDeleteFramebufferGLES2(NVGLUframebuffer* fb);

    void nvgluBindFramebufferGLES3(NVGLUframebuffer* fb);
    NVGLUframebuffer* nvgluCreateFramebufferGLES3(NVGcontext* ctx, int w, int h, int imageFlags);
    void nvgluDeleteFramebufferGLES3(NVGLUframebuffer* fb);
}
