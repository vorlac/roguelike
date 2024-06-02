#pragma once

#include "ds/vector2d.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct line
    {
        line(const point<T>& a, const point<T>& b)
            : coordinates{ a, b }
        {
        }

        point<T>& end()
        {
            return coordinates.back();
        }

        point<T>& start()
        {
            return coordinates.back();
        }

        std::vector<point<T>> coordinates{};
    };

#pragma pack()
}
