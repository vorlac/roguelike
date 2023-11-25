#pragma once

#include <array>
#include <limits>

#include <glad/gl.h>

#include "core/numeric_types.hpp"
#include "primitives/point.hpp"
#include "utils/concepts.hpp"

namespace rl::gl {
    template <rl::numeric T = rl::f32, auto Size = 1024 * sizeof(ds::point<f32>)>
        requires rl::positive_integer<Size>
    struct vertex_buffer
    {
        vertex_buffer()
        {
            glGenBuffers(1, &m_id);
            glBindBuffer(GL_ARRAY_BUFFER, m_id);  // sizeof(T) * m_buffer.size(), m_buffer.data());
        }

    private:
        constexpr static inline u32 INVALID_ID{ std::numeric_limits<u32>::max() };
        u32 m_id{ INVALID_ID };
        std::array<T, Size> m_buffer{};
    };
}
