#pragma once

#include <glad/gl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

#include "core/numeric_types.hpp"

namespace rl::gl {
    class Shader
    {
    public:
        u32 m_shader_id{ std::numeric_limits<u32>::max() };
        std::filesystem::path m_vertex_shader_path{};

        Shader(const char* vertexPath, const char* fragmentPath)
        {
            std::string vertex_shader_code{};
            std::string fragment_shader_code{};

            std::ifstream vertex_shader_fh{};
            std::ifstream fragment_shader_fh{};

            vertex_shader_fh.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fragment_shader_fh.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try
            {
                std::stringstream vShaderStream;
                std::stringstream fShaderStream;

                vertex_shader_fh.open(vertexPath);
                fragment_shader_fh.open(fragmentPath);

                vShaderStream << vertex_shader_fh.rdbuf();
                fShaderStream << fragment_shader_fh.rdbuf();

                vertex_shader_fh.close();
                fragment_shader_fh.close();

                vertex_shader_code = vShaderStream.str();
                fragment_shader_code = fShaderStream.str();
            }
            catch (std::ifstream::failure& e)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            }

            const char* vShaderCode = vertex_shader_code.c_str();
            const char* fShaderCode = fragment_shader_code.c_str();

            // vertex shader
            u32 vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);

            checkCompileErrors(vertex, "VERTEX");

            // fragment Shader
            u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);

            checkCompileErrors(fragment, "FRAGMENT");

            // shader Program
            m_shader_id = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            glLinkProgram(ID);

            checkCompileErrors(ID, "PROGRAM");

            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }

        void use() const
        {
            // activate shader
            glUseProgram(ID);
        }

        void setBool(const std::string& name, bool value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        }

        void setInt(const std::string& name, int value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }

        void setFloat(const std::string& name, float value) const
        {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }

        void setVec2(const std::string& name, const glm::vec2& value) const
        {
            glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        void setVec2(const std::string& name, float x, float y) const
        {
            glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
        }

        void setVec3(const std::string& name, const glm::vec3& value) const
        {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        void setVec3(const std::string& name, float x, float y, float z) const
        {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
        }

        void setVec4(const std::string& name, const glm::vec4& value) const
        {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        void setVec4(const std::string& name, float x, float y, float z, float w) const
        {
            glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
        }

        void setMat2(const std::string& name, const glm::mat2& mat) const
        {
            glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        void setMat3(const std::string& name, const glm::mat3& mat) const
        {
            glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        void setMat4(const std::string& name, const glm::mat4& mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

    private:
        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        void checkCompileErrors(GLuint shader, std::string type)
        {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                              << infoLog
                              << "\n -- --------------------------------------------------- -- "
                              << std::endl;
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                              << infoLog
                              << "\n -- --------------------------------------------------- -- "
                              << std::endl;
                }
            }
        }
    };
}
