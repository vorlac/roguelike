#pragma once

#include <cmath>
#include <limits>
#include <numbers>
#include <vector>

#include "ds/circle.hpp"
#include "ds/color.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "graphics/gl/shader.hpp"
#include "utils/math.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

namespace rl::gl {
    // OpenGL Vertex Buffer Object (VBO) representing a buffer of vertices that
    // openGL can read from when executing shaders on the data being rendered
    class InstancedVertexBuffer
    {
    public:
        enum class DrawMode {
            Fill,
            Wireframe,
        };

    public:
        static std::string name()
        {
            return "InstancedVertexBuffer";
        }

        explicit InstancedVertexBuffer(const ds::rect<f32>& viewport_rect)
        {
            scoped_log();

            auto&& window_rect{ viewport_rect.inflated(-450.0f) };

            // create vertex array object
            glGenVertexArrays(1, &m_vao_id);

            // create vertex buffer objects
            glGenBuffers(1, &m_vbo_id);
            glGenBuffers(1, &m_vbo_colors_id);
            glGenBuffers(1, &m_vbo_positions_id);

            // compile shaders
            bool shaders_valid = m_shader.compile();
            runtime_assert(shaders_valid, "Failed to compile shaders");

            ds::point<f32> centroid{ viewport_rect.centroid() };
            auto colors_size_mb = math::to_bytes(sizeof(f32) * 4 * m_rect_count, math::Units::Byte,
                                                 math::Units::Megabyte);
            auto positions_size_mb = math::to_bytes(sizeof(f32) * 3 * m_rect_count,
                                                    math::Units::Byte, math::Units::Megabyte);

            m_rect_colors_data.reserve(m_rect_count);
            m_rect_positions_data.reserve(m_rect_count);
            m_rect_velocities_data.reserve(m_rect_count);

            ds::color<u8> rect_color = ds::color<u8>::rand();

            diag_log("InstancedVertexBuffer Spawning {} Rectangles (clr:{}MB, pos:{}MB)",
                     m_rect_count, colors_size_mb, positions_size_mb);

            for (size_t i = 0; i < m_rect_count; ++i)
            {
                const f32 xv{ static_cast<f32>(rl::random<0U, 2000U>::value()) };
                const f32 yv{ static_cast<f32>(rl::random<0U, 2000U>::value()) };

                m_rect_colors_data.emplace_back((rl::random<0, 500>::value() + 250.0f) / 1000.0f,
                                                (rl::random<0, 500>::value() + 250.0f) / 1000.0f,
                                                (rl::random<0, 500>::value() + 250.0f) / 1000.0f);

                m_rect_velocities_data.emplace_back((xv - 1000.0f) / 10.0f, (yv - 1000.0f) / 10.0f);

                const ds::circle<f32> spawn{
                    viewport_rect.centroid(),
                    500.0f,
                };

                const f32 outer{ 500.0f };
                const f32 inner{ 250.0f };
                const f32 rand1{ (rand() % 1000) / 1000.0f };
                const f32 rand2{ (rand() % 1000) / 1000.0f };
                const f32 rad{ std::sqrt(rand1 * (outer * outer - inner * inner) + inner * inner) };

                const f32 theta{ f32(rand2 * 2.0f * std::numbers::pi) };

                m_rect_positions_data.emplace_back(spawn.centroid.x + rad * std::cos(theta),
                                                   spawn.centroid.y + rad * std::sin(theta));
            }
        }

        bool update_buffers(const ds::rect<f32>& viewport)
        {
            constexpr static auto top_bottom_collision = [](const ds::point<f32>& pos,
                                                            const ds::rect<f32>& window_rect) {
                const bool top_collision = pos.y <= 0.0f;
                const bool bottom_collision = pos.y + m_rect_size.height >= window_rect.size.height;
                return top_collision || bottom_collision;
            };

            constexpr static auto left_right_collision = [](const ds::point<f32>& pos,
                                                            const ds::rect<f32>& window_rect) {
                bool left_collision = pos.x <= 0.0f;
                bool right_collision = pos.x + m_rect_size.width >= window_rect.size.width;
                return left_collision || right_collision;
            };

            f32 delta_time = m_timer.delta();
            for (u32 i = 0; i < m_rect_count; ++i)
            {
                auto& pos = m_rect_positions_data[i];
                auto& vel = m_rect_velocities_data[i];

                pos.x += vel.x * delta_time;
                pos.y += vel.y * delta_time;
                if (left_right_collision(pos, viewport))
                    vel.x = -vel.x;
                if (top_bottom_collision(pos, viewport))
                    vel.y = -vel.y;
            }

            return true;
        }

        ~InstancedVertexBuffer()
        {
            // cleanup when everything leaves scope
            glDeleteVertexArrays(1, &m_vao_id);
            glDeleteBuffers(1, &m_vbo_positions_id);
            glDeleteBuffers(1, &m_vbo_colors_id);
            glDeleteBuffers(1, &m_vbo_id);
        }

        void set_draw_mode(const DrawMode mode = DrawMode::Fill) const
        {
            if (mode == m_draw_mode)
                return;

            switch (mode)
            {
                case DrawMode::Wireframe:
                    glPolygonMode(GL_FRONT, GL_LINE);
                    break;
                case DrawMode::Fill:
                    glPolygonMode(GL_FRONT, GL_FILL);
                    break;
            }
        }

        void bind_buffers()
        {
            // bind the VAO vertex array
            glBindVertexArray(m_vao_id);

            {
                // bind the VBO vertex buffer
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
                // define info about the VBO vertex buffer, targeting GL_ARRAY_BUFFER
                glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 3 * m_rect_vertex_buffer_data.size(),
                             m_rect_vertex_buffer_data.data(), GL_STATIC_DRAW);

                glVertexAttribPointer(     //
                    0,                     //
                    3,                     // index and count of vertices to configure
                    GL_FLOAT,              // data type of vertex data
                    GL_FALSE,              // should vertices be normalized (to 0 or -1 for signed)
                    3 * sizeof(f32),       // stride between each vertex record in the buffer
                    static_cast<void*>(0)  // void* offset of where the position data starts
                );
                glEnableVertexAttribArray(0);
            }

            {
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colors_id);
                // define info about the VBO vertex buffer, targeting GL_ARRAY_BUFFER
                glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 4 * m_rect_colors_data.size(),
                             m_rect_colors_data.data(), GL_STATIC_DRAW);

                glVertexAttribPointer(     //
                    1,                     //
                    4,                     //
                    GL_FLOAT,              //
                    GL_FALSE,              //
                    0,                     //
                    static_cast<void*>(0)  //
                );
                glEnableVertexAttribArray(1);
            }

            {
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions_id);
                // define info about the VBO vertex buffer, targeting GL_ARRAY_BUFFER
                glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 3 * m_rect_positions_data.size(),
                             m_rect_positions_data.data(), GL_DYNAMIC_DRAW);

                glVertexAttribPointer(     //
                    2,                     //
                    3,                     //
                    GL_FLOAT,              //
                    GL_FALSE,              //
                    0,                     //
                    static_cast<void*>(0)  //
                );
                glEnableVertexAttribArray(2);
            }

            this->set_draw_mode(DrawMode::Fill);
        }

        void draw_triangles()
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions_id);
            glBufferData(GL_ARRAY_BUFFER, m_rect_positions_data.size() * 3 * sizeof(f32),
                         m_rect_positions_data.data(), GL_STREAM_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colors_id);
            glBufferData(GL_ARRAY_BUFFER, m_rect_colors_data.size() * 4 * sizeof(f32),
                         m_rect_colors_data.data(), GL_STREAM_DRAW);

            glVertexAttribDivisor(0, 0);  // rect vertices: always reuse the same 4 vertices -> 0
            glVertexAttribDivisor(1, 1);  // rect colors: one per quad (its center) -> 1
            glVertexAttribDivisor(2, 1);  // rect positions: one per quad (its center) -> 1

            m_shader.set_active();

            glBindVertexArray(m_vao_id);
            glDrawArraysInstanced(GL_TRIANGLES, 0,
                                  static_cast<i32>(m_rect_vertex_buffer_data.size()), m_rect_count);

            glBindVertexArray(0);
        }

    private:
        rl::Timer<f32> m_timer{};
        DrawMode m_draw_mode{ DrawMode::Fill };
        Shader m_shader{ "instanced_vertex_shader.glsl", "instanced_fragment_shader.glsl" };

        constexpr static inline u32 m_rect_count{ 1000000 };
        constexpr static inline ds::dims<f32> m_rect_size{ 5.0f, 5.0f };
        constexpr static inline std::array<ds::point<f32>, 6> m_rect_vertex_buffer_data{
            ds::rect<f32>{
                ds::point<f32>{ 0.0f, 0.0f },
                ds::dims{ m_rect_size },
            }
                .triangles()
        };

        std::vector<ds::color<f32>> m_rect_colors_data{};
        std::vector<ds::point<f32>> m_rect_positions_data{};
        std::vector<ds::vector2<f32>> m_rect_velocities_data{};

        /**
         * @brief VBO name of fuffer containing
         * the rect vertices to use for instancing.
         * */
        u32 m_vbo_id{ std::numeric_limits<u32>::max() };
        /**
         * @brief VBO name of fuffer containing
         * the rect vertices to use for instancing.
         * */
        u32 m_vbo_colors_id{ std::numeric_limits<u32>::max() };

        /**
         * @brief VBO name of fuffer containing
         * the rect vertices to use for instancing.
         * */

        u32 m_vbo_positions_id{ std::numeric_limits<u32>::max() };

        /**
         * @brief Vertex Array Object ID used to idetnitfy
         * offsets/locations of vertex buffer attributes.
         * */
        u32 m_vao_id{ std::numeric_limits<u32>::max() };
    };
}
