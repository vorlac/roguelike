#pragma once

#include <glad/gl.h>

namespace rl::gl {
    struct OpenGL
    {
        struct Error
        {
            enum Code {
                InvalidEnum = GL_INVALID_ENUM,
                InvalidFrameBufferOP = GL_INVALID_FRAMEBUFFER_OPERATION,
                InvalidIndex = GL_INVALID_INDEX,
                InvalidOperation = GL_INVALID_OPERATION,
                InvalidValue = GL_INVALID_VALUE,
            };
        };
    };
}
