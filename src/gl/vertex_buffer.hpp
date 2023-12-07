#pragma once

#include <limits>
#include <vector>

#include "core/numeric.hpp"
#include "ds/color.hpp"
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

    public:
        VertexBuffer();
        ~VertexBuffer();

        void set_draw_mode(DrawMode mode);
        void draw_triangles(sdl::Window& window);
        void draw_rectangles(sdl::Window& window);

        /**
         * @brief Configure/define and bind all shared state between user app and openGL driver
         * */
        void bind_buffers(std::vector<std::pair<ds::point<f32>, ds::color<f32>>>& vbuff)
        {
            // bind the VAO vertex array
            glBindVertexArray(m_vao_id);

            if (!vbuff.empty())
            {
                // (3 floats per point + 4 floats per color) * vertex pair<point, color> count
                m_buffer_vertex_count = (3 + 4) * static_cast<i32>(vbuff.size());
                // bind the VBO vertex buffer
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
                // define info about the VBO vertex buffer, targeting GL_ARRAY_BUFFER
                glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * m_buffer_vertex_count, vbuff.data(),
                             GL_STATIC_DRAW);
            }

            glVertexAttribPointer(     //
                0, 3,                  // index and count of vertices to configure
                GL_FLOAT,              // data type of vertex data
                GL_FALSE,              // should vertices be normalized (to 0 or -1 for signed)
                7 * sizeof(f32),       // stride between each vertex record in the buffer
                static_cast<void*>(0)  // void* offset of where the position data starts
            );
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(                        //
                1, 4,                                     //
                GL_FLOAT,                                 //
                GL_FALSE,                                 //
                7 * sizeof(f32),                          //
                reinterpret_cast<void*>(3 * sizeof(f32))  //
            );
            glEnableVertexAttribArray(1);

            // note that this is allowed, the call to glVertexAttribPointer registered VBO
            // as the vertex attribute's bound vertex buffer object so afterwards we can
            // safely unbind
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // You can unbind the VAO afterwards so other VAO calls won't accidentally
            // modify this VAO, but this rarely happens. Modifying other VAOs requires a
            // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
            // VBOs) when it's not directly necessary.
            glBindVertexArray(0);

            this->set_draw_mode(DrawMode::Wireframe);
        }

    private:
        Shader m_shader{ "vertex_shader.glsl", "fragment_shader.glsl" };

        i32 m_buffer_vertex_count{ 0 };
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
