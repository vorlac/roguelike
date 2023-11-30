#pragma once

#include <glad/gl.h>

#include <concepts>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

#include "core/numeric_types.hpp"
#include "utils/assert.hpp"
#include "utils/io.hpp"

namespace rl::gl {

    enum class ShaderType : u32 {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER
    };

    struct ShaderProgram
    {
    private:
        template <auto T>
            requires std::same_as<decltype(T), ShaderType>
        struct Shader
        {
            ShaderType shader_type = T;
            using fspath = std::filesystem::path;
            const static inline fspath shaders_path{ std::filesystem::absolute(
                fspath("./data/shaders/").make_preferred()) };

            Shader() = default;

            Shader(std::filesystem::path glsl_path)
                : m_path{ std::filesystem::path(
                              shaders_path.generic_string() + glsl_path.generic_string())
                              .make_preferred() }
            {
                namespace fs = std::filesystem;
                bool glsl_exists = fs::exists(m_path);
                runtime_assert(glsl_exists, "GLSL file not found: {}", m_path);

                if (fs::exists(m_path))
                {
                    std::ifstream glsl(m_path);
                    m_glsl = { std::istreambuf_iterator<char>(glsl),
                               std::istreambuf_iterator<char>() };
                }
            }

            u32 compile()
            {
                log::info("Compiling shader: {}", m_path);
                u32 shader_id = glCreateShader(std::to_underlying(shader_type));
                const char* glsl = m_glsl.c_str();
                glShaderSource(shader_id, 1, &glsl, nullptr);
                glCompileShader(shader_id);

                i32 success = 0;
                glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
                if (success != 0)
                {
                    m_id = shader_id;
                    log::info("Success. Shader ID: {}", m_id);
                }
                else
                {
                    char error_msg[512] = { 0 };
                    glGetShaderInfoLog(shader_id, 512, NULL, error_msg);
                    runtime_assert(success != 0, "Shader ({}) compilation failed:\n{}",
                                   m_path.filename(), error_msg);
                }

                return m_id;
            }

            u32 id()
            {
                return m_id;
            }

        private:
            u32 m_id{ std::numeric_limits<u32>::max() };
            std::filesystem::path m_path{};
            std::string m_glsl{};
        };

    public:
        ShaderProgram(std::filesystem::path vertex_glsl_path = "vertex_shader.glsl",
                      std::filesystem::path fragment_glsl_path = "fragment_shader.glsl")
            : m_fragment_shader(fragment_glsl_path.native())
            , m_vertex_shader(vertex_glsl_path.native())
        {
        }

        inline bool compile()
        {
            u32 vert_shader_id = m_vertex_shader.compile();
            u32 frag_shader_id = m_fragment_shader.compile();
            if (vert_shader_id == 0 || frag_shader_id == 0)
                return false;

            m_shader_id = glCreateProgram();
            glAttachShader(m_shader_id, frag_shader_id);
            glAttachShader(m_shader_id, vert_shader_id);
            glLinkProgram(m_shader_id);

            i32 success = 0;
            log::info("Linking shaders...");
            glGetProgramiv(m_shader_id, GL_LINK_STATUS, &success);
            if (success != 0)
                log::info("Success. Shader Program ID: {}", m_shader_id);
            else
            {
                char error_msg[256] = { 0 };
                glGetProgramInfoLog(m_shader_id, 255, nullptr, error_msg);
                runtime_assert(success != 0, "Failed to build shader program (ID {}):\n{}",
                               m_shader_id, error_msg);
                return false;
            }

            glUseProgram(m_shader_id);

            glDeleteShader(vert_shader_id);
            glDeleteShader(frag_shader_id);

            return true;
        }

        u32 id()
        {
            return m_shader_id;
        }

    private:
        u32 m_shader_id{ std::numeric_limits<u32>::max() };
        Shader<ShaderType::Fragment> m_fragment_shader{};
        Shader<ShaderType::Vertex> m_vertex_shader{};
    };
}
