#pragma once

#include <glad/gl.h>

#include <string>

#include "core/numeric_types.hpp"
#include "utils/io.hpp"

namespace rl::gl {

    struct ShaderProgram
    {
        struct VertexShader
        {
            constexpr const static inline char* SOURCE =
                "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "void main()\n"
                "{\n"
                "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                "}\0";

            static inline u32 construct()
            {
                rl::u32 vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vert_shader_id, 1, &VertexShader::SOURCE, nullptr);
                glCompileShader(vert_shader_id);

                rl::i32 success = 0;
                glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &success);
                if (success == 0)
                {
                    char error_msg[512] = { 0 };
                    glGetShaderInfoLog(vert_shader_id, 512, NULL, error_msg);
                    log::error("Vertex shader compilation failed: {}", error_msg);
                }

                return success != 0 ? vert_shader_id : 0;
            }
        };

        struct FragmentShader
        {
            constexpr const static inline char* SOURCE =
                "#version 330 core \n"
                "out vec4 FragColor; \n"
                "void main() \n"
                "{\n"
                "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                "}\0";

            static inline u32 construct()
            {
                rl::u32 frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(frag_shader_id, 1, &FragmentShader::SOURCE, nullptr);
                glCompileShader(frag_shader_id);

                rl::i32 success = 0;
                glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &success);
                if (success == 0)
                {
                    char error_msg[512] = { 0 };
                    glGetShaderInfoLog(frag_shader_id, 512, NULL, error_msg);
                    log::error("Fragment shader compilation failed: {}", error_msg);
                }

                return success != 0 ? frag_shader_id : 0;
            }
        };

        static inline bool build()
        {
            u32 vert_shader_id = VertexShader::construct();
            u32 frag_shader_id = FragmentShader::construct();
            if (vert_shader_id == 0 || frag_shader_id == 0)
                return false;

            u32 shader_id = glCreateProgram();
            glAttachShader(shader_id, frag_shader_id);
            glAttachShader(shader_id, vert_shader_id);
            glLinkProgram(shader_id);

            i32 success = 0;
            glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
            if (success == 0)
            {
                char error_msg[256] = { 0 };
                glGetProgramInfoLog(shader_id, 255, nullptr, error_msg);
                log::error("Failed to build shader program: {}", error_msg);
                return false;
            }

            glUseProgram(shader_id);
            glDeleteShader(vert_shader_id);
            glDeleteShader(frag_shader_id);

            return true;
        }
    };
}
