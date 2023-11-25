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
        glGenVertexArrays(1, &m_vao_id);
        glGenBuffers(1, &m_vbo_id);
        m_shader_id = m_shader.build();
        // std::memmove(m_buffer.data(), vertices, count * 12);
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteVertexArrays(1, &m_vao_id);
        glDeleteBuffers(1, &m_vbo_id);
        glDeleteProgram(m_shader_id);
    }

    void VertexBuffer::assign_shaders(u32 shader_id)
    {
        // define the shader program to use for rendering
        glUseProgram(shader_id);
    }

    void VertexBuffer::enable_wireframe_mode()
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    //// bind the VAO memory buffer
    // void VertexBuffer::bind_vbo(auto&& buff)
    //{
    //     glBindVertexArray(m_vao_id);
    //     glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);

    //    static std::vector<float> test = { buff.data()->data(), buff.data()->data() + 36 };

    //    glBufferData(GL_ARRAY_BUFFER, buff.size() * sizeof(f32) * 9, buff.data(), GL_STATIC_DRAW);
    //    glVertexAttribPointer(     //
    //        0,                     // index of vertex to configure
    //        3,                     // size of the vertex (point count)
    //        GL_FLOAT,              // data type of vertex data
    //        GL_FALSE,              // should vertices be normalized (to 0 or -1 for signed)
    //        3 * sizeof(float),     // stride between each vertex record in the buffer
    //        static_cast<void*>(0)  // void* offset of where the position data starts
    //    );
    //    glEnableVertexAttribArray(0);

    //    // note that this is allowed, the call to glVertexAttribPointer registered VBO
    //    // as the vertex attribute's bound vertex buffer object so afterwards we can
    //    // safely unbind
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //    // You can unbind the VAO afterwards so other VAO calls won't accidentally
    //    // modify this VAO, but this rarely happens. Modifying other VAOs requires a
    //    // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
    //    // VBOs) when it's not directly necessary.
    //    glBindVertexArray(0);
    //}

    void VertexBuffer::draw(sdl::Window&)
    {
        glUseProgram(m_shader_id);
        // seeing as we only have a single VAO there's no need to bind
        // it every time, but we'll do so to keep things a bit more organized
        glBindVertexArray(m_vao_id);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // no need to unbind it every time
        // glBindVertexArray(0);
    }
}
