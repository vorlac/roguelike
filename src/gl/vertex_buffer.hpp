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
        enum class DrawMode {
            Fill,
            Wireframe,
        };

        constexpr static inline unsigned int indices[] = {
            // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };
        constexpr static inline int test = sizeof(indices);

    public:
        VertexBuffer();
        ~VertexBuffer();

        void set_draw_mode(DrawMode mode);
        void draw_triangles(sdl::Window& window);
        void draw_rectangles(sdl::Window& window);

        /**
         * @brief Configure/define and bind all shared state between user app and openGL driver
         * */
        void bind_buffers(auto& vbuff, auto& ibuff)
        {
            // bind the VAO vertex array
            glBindVertexArray(m_vao_id);

            if (!vbuff.empty())
            {
                // bind the VBO vertex buffer
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
                // define info about the VBO vertex buffer, targeting GL_ARRAY_BUFFER
                glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 3 * vbuff.size(), vbuff.data(),
                             GL_STATIC_DRAW);
            }

            if (!ibuff.empty())
            {
                // bind the EBO index buffer using
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_id);
                // define info about the EBO index buffer, targeting GL_ELEMENT_ARRAY_BUFFER
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * ibuff.size(), ibuff.data(),
                             GL_STATIC_DRAW);
            }

            glVertexAttribPointer(     //
                0, 3,                  // index and count of vertices to configure
                GL_FLOAT,              // data type of vertex data
                GL_FALSE,              // should vertices be normalized (to 0 or -1 for signed)
                3 * sizeof(f32),       // stride between each vertex record in the buffer
                static_cast<void*>(0)  // void* offset of where the position data starts
            );

            glEnableVertexAttribArray(0);

            // note that this is allowed, the call to glVertexAttribPointer registered VBO
            // as the vertex attribute's bound vertex buffer object so afterwards we can
            // safely unbind
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // // Do !NOT! unbind the EBO while a VAO is active as the bound element
            // // buffer object IS stored in the VAO; keep the EBO bound.
            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            // You can unbind the VAO afterwards so other VAO calls won't accidentally
            // modify this VAO, but this rarely happens. Modifying other VAOs requires a
            // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
            // VBOs) when it's not directly necessary.
            glBindVertexArray(0);

            this->set_draw_mode(DrawMode::Wireframe);
        }

    private:
        i32 m_shader_id{ std::numeric_limits<i32>::max() };

        /**
         * @brief Vertex Buffer Object ID
         *
         * The VBO manages the buffer of vertices
         * shared with the GPU / shaders to be rendered
         * */
        u32 m_vbo_id{ std::numeric_limits<u32>::max() };

        /**
         * @brief Vertex Array Object ID
         *
         * The VAO manages the info that identifies where
         * certain data/polygons/properties exist in the
         * VBO vertex buffer.
         * */
        u32 m_vao_id{ std::numeric_limits<u32>::max() };

        /**
         * @brief Element Buffer Object ID
         *
         * The EBO manages index mappings into the VBO
         * that allows for vertex compression if it can
         * be arranged with overlapping ranges for
         * adjacent data
         * */
        u32 m_ebo_id{ std::numeric_limits<u32>::max() };
    };
}
