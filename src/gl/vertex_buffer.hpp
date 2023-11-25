#pragma once

#include <limits>
#include <vector>

#include "core/numeric_types.hpp"
#include "ds/point.hpp"
#include "ds/triangle.hpp"
#include "ds/vector2d.hpp"
#include "gl/shader.hpp"
#include "sdl/window.hpp"

namespace rl::gl {
    /**
     * @brief OpenGL Vertex Buffer Object (VBO) representing a buffer of vertices
     * that openGL can read from when executing shaders on the data being rendered
     * */
    class VertexBuffer
    {
    public:
        VertexBuffer();
        ~VertexBuffer();

        void enable_wireframe_mode();
        void assign_shaders(u32 shader_id);
        // void bind_vbo(auto&& buff);
        void draw(sdl::Window& window);

        // bind the VAO memory buffer
        void bind_vbo(auto&& buff)
        {
            glBindVertexArray(m_vao_id);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);

            glBufferData(GL_ARRAY_BUFFER, buff.size() * sizeof(buff), buff.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(     //
                0,                     // index of vertex to configure
                3,                     // size of the vertex (point count)
                GL_FLOAT,              // data type of vertex data
                GL_FALSE,              // should vertices be normalized (to 0 or -1 for signed)
                3 * sizeof(float),     // stride between each vertex record in the buffer
                static_cast<void*>(0)  // void* offset of where the position data starts
            );
            glEnableVertexAttribArray(0);

            // note that this is allowed, the call to glVertexAttribPointer registered VBO
            // as the vertex attribute's bound vertex buffer object so afterwards we can
            // safely unbind
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // You can unbind the VAO afterwards so other VAO calls won't accidentally
            // modify this VAO, but this rarely happens. Modifying other VAOs requires a
            // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
            // VBOs) when it's not directly necessary.
            glBindVertexArray(0);
        }

    private:
        ShaderProgram m_shader{};
        i32 m_shader_id{ std::numeric_limits<i32>::max() };
        u32 m_vbo_id{ std::numeric_limits<u32>::max() };
        u32 m_vao_id{ std::numeric_limits<u32>::max() };
    };
}
