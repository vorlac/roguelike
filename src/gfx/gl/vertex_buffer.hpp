#pragma once

#include <limits>
#include <utility>
#include <vector>

#include "core/main_window.hpp"
#include "ds/color.hpp"
#include "ds/point.hpp"
#include "ds/triangle.hpp"
#include "ds/vector2d.hpp"
#include "gfx/gl/shader.hpp"
#include "utils/numeric.hpp"

namespace rl::gl {
    /**
     * @brief OpenGL Vertex Buffer Object (VBO) representing a buffer of vertices
     * that openGL can read from when executing shaders on the data being rendered
     * */
    class VertexBuffer {
    public:
        enum class DrawMode {
            Fill,
            Wireframe,
        };

    public:
        VertexBuffer(const ds::rect<f32>& viewport_rect) {
            // bind vertex array object
            glGenVertexArrays(1, &m_vao_id);
            // create vertex buffer object
            glGenBuffers(1, &m_vbo_id);

            // compile shaders
            bool shaders_valid = m_shader.compile();
            debug_assert(shaders_valid, "Failed to compile shaders");
        }

        ~VertexBuffer() {
            // cleanup when everything leaves scope
            glDeleteVertexArrays(1, &m_vao_id);
            glDeleteBuffers(1, &m_vbo_id);
            glDeleteBuffers(1, &m_vbo_id);
        }

        // Defaults to fill
        void set_draw_mode(DrawMode mode = DrawMode::Fill) {
            switch (mode) {
                case DrawMode::Wireframe:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    break;
                case DrawMode::Fill:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    break;
            }
        }

        /**
         * @brief Configure/define and bind all shared
         * buffers between application and openGL API
         * */
        void bind_buffers() {
            // bind the VAO vertex array
            glBindVertexArray(m_vao_id);

            if (!m_rects.empty()) {
                // (3 floats per point + 4 floats per color) * vertex pair<point, color> count
                m_buffer_vertex_count = (3 + 4) * 6 * static_cast<i32>(m_rects.size());
                // bind the VBO vertex buffer
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
                // define info about the VBO vertex buffer, targeting GL_ARRAY_BUFFER
                glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * m_buffer_vertex_count,
                             m_rects.data(), GL_STATIC_DRAW);
            }

            glVertexAttribPointer(
                0, 3,                  // index and count of vertices to configure
                GL_FLOAT,              // data type of vertex data
                GL_FALSE,              // should vertices be normalized (to 0 or -1 for signed)
                7 * sizeof(f32),       // stride between each vertex record in the buffer
                static_cast<void*>(0)  // void* offset of where the position data starts
            );
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(
                1, 4,
                GL_FLOAT,
                GL_FALSE,
                7 * sizeof(f32),
                reinterpret_cast<void*>(3 * sizeof(f32)));

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

        void draw_triangles() {
            // Set shader program to use
            m_shader.set_active();
            // glUseProgram(m_shader_id);
            //  Bind VAO array
            glBindVertexArray(m_vao_id);

            // Draw verices
            // GL_QUADS, GL_TRIANGLE_STRIP
            glDrawArrays(GL_TRIANGLES, 0, m_buffer_vertex_count);

            // Unbind the VAO buffer... not necessary yet
            // glBindVertexArray(0);
        }

    private:
        Shader m_shader{ "vertex_shader.glsl", "fragment_shader.glsl" };

        static inline const std::array quads{
            ds::rect<f32>{ { 0.0f, 0.0f }, { 1920.0f, 1080.0f } }.quads(),
        };
        static inline const std::array m_rects{
            ds::rect<f32>{ quads[0].expanded(-50.0f) }.triangles(rl::Colors::Red),
            ds::rect<f32>{ quads[1].expanded(-50.0f) }.triangles(rl::Colors::Blue),
            ds::rect<f32>{ quads[2].expanded(-50.0f) }.triangles(rl::Colors::Purple),
            ds::rect<f32>{ quads[3].expanded(-50.0f) }.triangles(rl::Colors::Green),
        };

        i32 m_buffer_vertex_count{ 0 };
        i32 m_shader_id{ std::numeric_limits<i32>::max() };

        /**
         * @brief Vertex Buffer Object ID
         *
         * The VBO manages the buffer of vertices
         * shared with the GPU / shaders to be rendered
         * */
        u32 m_vbo_id{ std::numeric_limits<u32>::max() };
        u32 m_vbo_colors_id{ std::numeric_limits<u32>::max() };
        u32 m_vbo_positions_id{ std::numeric_limits<u32>::max() };
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
