#pragma once

#include <glad/gl.h>

#include <concepts>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <type_traits>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>

#include "core/assert.hpp"
#include "utils/concepts.hpp"
#include "utils/fs.hpp"
#include "utils/io.hpp"
#include "utils/numeric.hpp"

namespace rl::gl {

    struct Shader
    {
        enum class Program : u32 {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER
        };

        template <auto VShaderType>
            requires std::same_as<decltype(VShaderType), Program>
        struct GLSL
        {
            constexpr static inline Program shader_type = VShaderType;
            static inline const std::filesystem::path GLSL_SHADER_DIR = {
                fs::absolute("shaders/"),
            };

            GLSL() = default;

            GLSL(std::filesystem::path glsl_path)
                : m_path{ fs::absolute(GLSL_SHADER_DIR.generic_string() + glsl_path.generic_string())
                              .make_preferred() }
            {
                namespace fs = std::filesystem;
                bool glsl_exists = fs::exists(m_path);
                runtime_assert(glsl_exists, "GLSL file not found: {}", m_path);

                if (fs::exists(m_path))
                {
                    std::ifstream glsl(m_path);
                    m_glsl = std::string{ std::istreambuf_iterator<char>(glsl),
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
        Shader(std::filesystem::path vert_glsl_file, std::filesystem::path frag_glsl_file)
            : m_fragment_shader(frag_glsl_file.native())
            , m_vertex_shader(vert_glsl_file.native())
        {
        }

        ~Shader()
        {
            glDeleteProgram(m_shader_id);
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

        u32 id() const
        {
            return m_shader_id;
        }

        void set_active()
        {
            glUseProgram(m_shader_id);
            this->set_transform();
        }

        template <typename T>
            requires std::same_as<T, bool>
        void set_value(auto&& name, T value) const
        {
            glUniform1i(glGetUniformLocation(m_shader_id, name.data()), value ? 1 : 0);
        }

        template <typename T>
            requires rl::any_of<T, f32, i32, u32>
        void set_value(auto&& name, T value) const
        {
            glUniform1i(glGetUniformLocation(m_shader_id, name.data()), value);
        }

        void set_transform()
        {
            glm::mat4 model = glm::identity<glm::mat4>();
            glm::mat4 view = glm::identity<glm::mat4>();
            glm::mat4 projection = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f, 0.1f, 100.0f);

            model = glm::scale(model, glm::vec3(1.0f));
            model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

            auto mvp{ projection * view * model };
            u32 mvp_loc = glGetUniformLocation(m_shader_id, "mvp");
            glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
        }

    private:
        u32 m_shader_id{ std::numeric_limits<u32>::max() };
        GLSL<Shader::Program::Fragment> m_fragment_shader{};
        GLSL<Shader::Program::Vertex> m_vertex_shader{};
    };
}
