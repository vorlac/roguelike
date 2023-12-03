#pragma once

#include <glad/gl.h>

#include <vector>

#include "core/numeric.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "sdl/time.hpp"

namespace rl::gl {
    struct InstancedVertexArray
    {
        InstancedVertexArray(const ds::rect<f32>& viewport_rect)
        {
            glGenBuffers(1, &m_vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 3 * m_rect_vertex_buffer_data.size(),
                         m_rect_vertex_buffer_data.data(), GL_STATIC_DRAW);

            // The VBO containing the positions and sizes of
            // the particles. Init with null buffer, it will
            // be updated later, once per frame.
            glGenBuffers(1, &m_posize_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_posize_buffer);
            glBufferData(GL_ARRAY_BUFFER, m_rect_count * 4 * sizeof(f32), nullptr, GL_STREAM_DRAW);

            // The VBO containing the colors of the particles
            // Init with null buffer, it will be updated later
            glGenBuffers(1, &m_colors_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, m_colors_buffer);
            glBufferData(GL_ARRAY_BUFFER, m_rect_count * 4 * sizeof(u8), nullptr, GL_STREAM_DRAW);

            bool success = m_shader.compile();
            runtime_assert(success, "invalid shader, compilation failed");

            ds::point<f32> centroid = viewport_rect.centroid();
            m_rect_colors_buffer_data.reserve(m_rect_count);
            m_rect_velocities.reserve(m_rect_count);

            srand((u32)time(nullptr));
            auto rect_color = ds::color<u8>{
                static_cast<u8>(rand() % 128),
                static_cast<u8>(rand() % 128),
                static_cast<u8>(rand() % 128),
            };

            for (size_t i = 0; i < m_rect_count; ++i)
            {
                u32 xv = rand() % 2000;
                u32 yv = rand() % 2000;

                m_rect_colors_buffer_data.emplace_back(static_cast<u8>(rand() % 128),
                                                       static_cast<u8>(rand() % 128),
                                                       static_cast<u8>(rand() % 128));

                m_rect_velocities.emplace_back(static_cast<f32>((xv - 1000.0f) / 10.0f),
                                               static_cast<f32>((yv - 1000.0f) / 10.0f));

                m_rect_pos_sizes_buffer_data.emplace_back(std::forward<ds::point<f32>>(centroid),
                                                          m_rect_size);
            }
        }

        bool bind_buffers()
        {
            // 1rst attribute buffer:
            // rect vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
            glVertexAttribPointer(     //
                0,                     // attribute, layout = 0 in the shader
                3,                     // size
                GL_FLOAT,              // type
                GL_FALSE,              // normalized?
                0,                     // stride
                static_cast<void*>(0)  // array buffer offset
            );

            // 2nd attribute buffer:
            // rect centroids & sizes
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, m_posize_buffer);
            glVertexAttribPointer(     //
                1,                     // attribute,layout = 1 in the shader.
                4,                     // size : x + y + z + size => 4
                GL_FLOAT,              // type
                GL_FALSE,              // normalized?
                0,                     // stride
                static_cast<void*>(0)  // array buffer offset
            );

            // 3rd attribute buffer:
            // rect colors
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, m_colors_buffer);
            glVertexAttribPointer(     //
                2,                     // attribute, layout = 2 in the shader
                4,                     // size : r + g + b + a => 4
                GL_UNSIGNED_BYTE,      // type
                GL_TRUE,               // normalized? yes, this means the u8[4] will be accessible
                                       //             with a vec4(f32) in the shader
                0,                     // stride
                static_cast<void*>(0)  // array buffer offset
            );

            return true;
        }

        bool update_buffers(const ds::dims<i32>& window_size)
        {
            static auto top_bottom_collision = [&](const ds::point<f32>& pos) {
                bool top_collision = pos.y - (m_rect_size / 2.0f) <= 0.0f;
                bool bottom_collision = pos.y + (m_rect_size / 2.0f) >= window_size.height;
                return top_collision || bottom_collision;
            };
            static auto left_right_collision = [&](const ds::point<f32>& pos) {
                bool left_collision = pos.x - (m_rect_size / 2.0f) <= 0.0f;
                bool right_collision = pos.x + (m_rect_size / 2.0f) >= window_size.width;
                return left_collision || right_collision;
            };

            f32 delta_time = m_timer.delta();
            for (u32 i = 0; i < m_rect_count; ++i)
            {
                auto& [pos, siz] = m_rect_pos_sizes_buffer_data[i];
                auto& vel = m_rect_velocities[i];

                pos.x += vel.x * delta_time;
                pos.y += vel.y * delta_time;
                if (left_right_collision(pos))
                    vel.x = -vel.x;
                if (top_bottom_collision(pos))
                    vel.y = -vel.y;

                // TODO: APPLY WORLD SPACE TRANSFORM WITH CONST RECT HERE???
                m_shader.set_transform(pos);
            }

            // http://www.opengl.org/wiki/Buffer_Object_Streaming
            glBindBuffer(GL_ARRAY_BUFFER, m_posize_buffer);
            glBufferData(GL_ARRAY_BUFFER, m_rect_count * 4 * sizeof(f32), nullptr, GL_STREAM_DRAW);

            // todo: use for buffer orphaning later if/when the
            // count isn't always going to be m_rect_count
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_rect_count * sizeof(f32) * 4,
                            m_rect_pos_sizes_buffer_data.data());

            glBindBuffer(GL_ARRAY_BUFFER, m_colors_buffer);
            glBufferData(GL_ARRAY_BUFFER, m_rect_count * 4 * sizeof(u8), nullptr, GL_STREAM_DRAW);

            // todo: use for buffer orphaning later if/when the
            // count isn't always going to be m_rect_count
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_rect_count * sizeof(u8) * 4,
                            m_rect_colors_buffer_data.data());

            return true;
        }

        bool render_buffers()
        {
            m_shader.set_active();

            // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
            glVertexAttribDivisor(0, 0);  // rect vertices: always reuse the same 4 vertices -> 0
            glVertexAttribDivisor(1, 1);  // rect positions: one per quad (its center) -> 1
            glVertexAttribDivisor(2, 1);  // rect colors: one per quad -> 1

            // draw the vertices from m_rect_vertex_buffer_data, m_rect count time
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_rect_count);

            return true;
        }

        // r + g + b + a = sizeof(float) * 4
        std::vector<ds::color<u8>> m_rect_colors_buffer_data{};
        // x + y + z + size = sizeof(float) * 4
        std::vector<std::pair<ds::point<f32>, f32>> m_rect_pos_sizes_buffer_data{};
        // only used for position updates
        std::vector<ds::vector2<f32>> m_rect_velocities{};

        constexpr static inline u32 m_rect_count{ 5 };
        constexpr static inline f32 m_rect_size{ 15.0f };

        // single rect's vertices that will be reused to draw all rects
        constexpr static inline std::array m_rect_vertex_buffer_data{
            ds::triangle<f32>{ { -0.5f, -0.5f }, { 0.5f, -0.5 }, { 0.0f, 0.5f } }.points()
        };

        u32 m_vertex_buffer{ 999 };
        u32 m_colors_buffer{ 999 };
        u32 m_posize_buffer{ 999 };

        gl::Shader m_shader{};
        sdl::Timer<f32> m_timer{};
    };
}
