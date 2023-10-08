#pragma once

#include "math/point2d.hpp"
#include "math/vector2d.hpp"

#include <fmt/format.h>
#include <locale>
#include <string>

namespace rl::io
{
    const static std::locale locale = std::locale("en_US.UTF-8");

    template <typename T>
    concept 2DScalar = requires(T t) {
        t.x;
        t.y;
    };

    constexpr std::string to_string(const vector2<T>& vec)
    {
        return fmt::format("[{}, {}]", vec.x, vec.y);
    }
}
