#include <glad/gl.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics/vg/nanovg_gl_utils.hpp"

// FBO is core in OpenGL 3>.
#define NANOVG_FBO_VALID 1
#ifndef NANOVG_GL3
  #define NANOVG_GL3
#endif

namespace rl::vg {
    static GLint defaultFBO = -1;

    NVGLUframebuffer* nvgluCreateFramebufferGL3(NVGcontext* ctx, int w, int h, int imageFlags)
    {
        // GLint defaultFBO;
        GLint defaultRBO;
        NVGLUframebuffer* fb = NULL;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &defaultRBO);

        fb = (NVGLUframebuffer*)malloc(sizeof(NVGLUframebuffer));
        if (fb == NULL)
            goto error;
        memset(fb, 0, sizeof(NVGLUframebuffer));

        fb->image = nvgCreateImageRGBA(
            ctx, w, h, imageFlags | NVG_IMAGE_FLIPY | NVG_IMAGE_PREMULTIPLIED, NULL);

        fb->texture = nvglImageHandleGL3(ctx, fb->image);
        fb->ctx = ctx;

        // frame buffer object
        glGenFramebuffers(1, &fb->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

        // render buffer object
        glGenRenderbuffers(1, &fb->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h);

        // combine all
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->texture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
#ifdef GL_DEPTH24_STENCIL8
            // If GL_STENCIL_INDEX8 is not supported, try GL_DEPTH24_STENCIL8 as a fallback.
            // Some graphics cards require a depth buffer along with a stencil.
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->texture,
                                   0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      fb->rbo);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
#endif  // GL_DEPTH24_STENCIL8
                goto error;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, defaultRBO);
        return fb;
    error:
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, defaultRBO);

        nvgluDeleteFramebufferGL3(fb);

        return NULL;
    }

    void nvgluBindFramebufferGL3(NVGLUframebuffer* fb)
    {
        if (defaultFBO == -1)
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, fb != NULL ? fb->fbo : defaultFBO);
    }

    void nvgluDeleteFramebufferGL3(NVGLUframebuffer* fb)
    {
        if (fb == NULL)
            return;
        if (fb->fbo != 0)
            glDeleteFramebuffers(1, &fb->fbo);
        if (fb->rbo != 0)
            glDeleteRenderbuffers(1, &fb->rbo);
        if (fb->image >= 0)
            nvgDeleteImage(fb->ctx, fb->image);
        fb->ctx = NULL;
        fb->fbo = 0;
        fb->rbo = 0;
        fb->texture = 0;
        fb->image = -1;
        free(fb);
    }
}
