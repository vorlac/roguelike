#pragma once

#include <string>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>

#include "core/numeric.hpp"
#include "ds/point.hpp"
#include "ds/triangle.hpp"
#include "gl/shader.hpp"
#include "utils/concepts.hpp"

namespace rl::math {
    inline f32 inverse_lerp(f32 from, f32 to, f32 val)
    {
        return (val - from) / (to - from);
    }

    inline void create_transformations(gl::Shader& shader, f32 width, f32 height)
    {
        // create transformations
        // make sure to initialize matrix to identity matrix first
        glm::mat4 view = glm::identity<glm::mat4>();
        glm::mat4 proj = glm::identity<glm::mat4>();

        // set projection perpective matrix from angle, aspect ratio, and close/far vals
        proj = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
        // view from 3 units above??
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        // pass transformation matrices to the shader
        shader.setMat4("projection", proj);  // note: currently we set the projection
                                             // matrix each frame, but since the projection
                                             // matrix rarely changes it's often best
                                             // practice to set it outside the main loop
                                             // only once.
        shader.setMat4("view", view);
    }
}
