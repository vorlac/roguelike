#pragma once

#include "core/ds/point.hpp"

namespace rl::ui
{
    /**
     * @brief Represents a location that all controls
     *        will use to position from relatively
     * */
    class Anchor
    {
    public:
        Anchor(float x = 0.0f, float y = 0.0f)
            : m_pt{ x, y }
        {
        }

    private:
        ds::point<float> m_pt{ 0, 0 };
    };
}
