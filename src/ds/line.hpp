#pragma once

#include "ds/vector2d.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct line {
        point<T> start{ point<T>::null() };
        point<T> end{ point<T>::null() };
    };

#pragma pack()
}
