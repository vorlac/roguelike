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
#include <glm/matrix.hpp>

#include "core/numeric.hpp"
#include "utils/assert.hpp"
#include "utils/concepts.hpp"
#include "utils/fs.hpp"
#include "utils/io.hpp"

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
            const static inline std::filesystem::path GLSL_SHADER_DIR = {
                fs::absolute("data/shaders/"),
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
        Shader(std::filesystem::path vert_glsl_file = "vertex_shader.glsl",
               std::filesystem::path frag_glsl_file = "fragment_shader.glsl")
            : m_fragment_shader(frag_glsl_file.native())
            , m_vertex_shader(vert_glsl_file.native())
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

        void set_active()
        {
            glUseProgram(m_shader_id);
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

        void set_transform(auto&& vert)
        {
            // The vertical FoV (radians), amount of zoom
            constexpr float fov = -55.0f;
            // Camera is at (4,3,3), in world space
            glm::vec3 camera_pos(0, 0, 0);
            // wheree the camera should look (origin)
            glm::vec3 camera_target(0, 0, 0);
            // the vector pointing in the up direction in the world
            glm::vec3 up_direction(0, 1, 0);

            // origin / identity matrix
            glm::mat4 model_matrix = glm::identity<glm::mat4>();

            // where the view should be updated, 3 units to the right (world moves -3)
            // ... glm::vec3(-3.0f, 0.0f, 0.0f), for now don't move the campera...
            glm::mat4 view_matrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));

            glm::mat4 camera_matrix = glm::lookAt(  //
                camera_pos,                         // camera location in world space
                camera_target,                      // where camera should be aiming in world space
                up_direction                        // up, (0,-1,0) would make the view upside-down
            );

            glm::mat4 projection_matrix = glm::perspective(
                glm::radians(fov),  // Think "camera lens", usually 90(wide) - 30(zoomed in)
                1920.0f / 1080.0f,  // Aspect Ratio. Depends on window dimensions.
                0.1f,   // Near clipping plane, keep as big as possible to avoid precision issues.
                100.0f  // Far clipping plane. Keep as little as possible.
            );

            // retrieve the matrix uniform locations
            u32 model_loc = glGetUniformLocation(m_shader_id, "model");
            u32 view_loc = glGetUniformLocation(m_shader_id, "view");
            u32 proj_loc = glGetUniformLocation(m_shader_id, "projection");

            u32 vert_loc = glGetUniformLocation(m_shader_id, "velocity");

            // Get a handle for our "MVP" uniform
            // Only during the initialisation

            // Send our transformation to the currently bound shader, in the "MVP" uniform
            // This is done in the main loop since each model will have a different MVP matrix (At
            // least for the M part)
            glUniformMatrix4fv(vert_loc, 1, GL_FALSE,
                               glm::value_ptr(glm::vec3(vert.x, vert.y, vert.z)));

            // pass them to the shaders (3 different ways)
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
            glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(camera_matrix));
            glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        }

    private:
        u32 m_shader_id{ std::numeric_limits<u32>::max() };
        GLSL<Shader::Program::Fragment> m_fragment_shader{};
        GLSL<Shader::Program::Vertex> m_vertex_shader{};
    };
}
