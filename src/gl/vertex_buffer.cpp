#include <glad/gl.h>

#include <cstring>
#include <vector>

#include "core/numeric.hpp"
#include "ds/triangle.hpp"
#include "gl/vertex_buffer.hpp"
#include "utils/assert.hpp"

namespace rl::gl {
    VertexBuffer::VertexBuffer()
    {
        glEnable(GL_DEBUG_OUTPUT);
        // bind vertex array object
        glGenVertexArrays(1, &m_vao_id);
        // create vertex buffer object
        glGenBuffers(1, &m_vbo_id);

        // compile shaders
        Shader m_shader{};
        if (m_shader.compile())
        {
            m_shader_id = m_shader.id();
            // m_shader.set_transform();
        }
    }

    VertexBuffer::~VertexBuffer()
    {
        // cleanup when everything leaves scope
        glDeleteVertexArrays(1, &m_vao_id);
        glDeleteBuffers(1, &m_vbo_id);
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
        glDrawArrays(GL_TRIANGLES, 0, m_buffer_vertex_count);

        // Unbind the VAO buffer... not necessary yet
        // glBindVertexArray(0);
    }
}
