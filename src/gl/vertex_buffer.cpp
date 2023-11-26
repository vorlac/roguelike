#include <glad/gl.h>

#include <cstring>
#include <vector>

#include "core/numeric_types.hpp"
#include "ds/triangle.hpp"
#include "gl/vertex_buffer.hpp"
#include "utils/assert.hpp"

namespace rl::gl {
    VertexBuffer::VertexBuffer()
    {
        // glEnable(GL_DEBUG_OUTPUT);
        // bind vertex array object
        glGenVertexArrays(1, &m_vao_id);
        // create vertex buffer object
        glGenBuffers(1, &m_vbo_id);
        // create element buffer object
        glGenBuffers(1, &m_ebo_id);

        // compile shaders
        ShaderProgram m_shader{};
        m_shader_id = m_shader.build();
    }

    VertexBuffer::~VertexBuffer()
    {
        // cleanup when everything leaves scope
        glDeleteVertexArrays(1, &m_vao_id);
        glDeleteBuffers(1, &m_vbo_id);
        glDeleteBuffers(1, &m_ebo_id);
        glDeleteProgram(m_shader_id);
    }

    // Defaults to fill
    void VertexBuffer::set_draw_mode(DrawMode mode = DrawMode::Fill)
    {
        switch (mode)
        {
            case DrawMode::Wireframe:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case DrawMode::Fill:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
        }
    }

    void VertexBuffer::draw_triangles(sdl::Window&)
    {
        // Set shader program to use
        glUseProgram(m_shader_id);
        // Bind VAO array
        glBindVertexArray(m_vao_id);

        // Draw verices
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Unbind the VAO buffer... not necessary yet
        // glBindVertexArray(0);
    }

    void VertexBuffer::draw_rectangles(sdl::Window&)
    {
        // Set shader program to use
        glUseProgram(m_shader_id);

        // Bind VAO array
        glBindVertexArray(m_vao_id);

        // Draw rects
        // Instead of calling draw arrays (like ni draw_triangles)
        // this draws the vertices as objects from the buffer
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Unbind the VAO buffer... not necessary yet
        // glBindVertexArray(0);
    }
}
