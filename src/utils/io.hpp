#pragma once

#include <concepts>
#include <locale>
#include <string>
#include <fmt/format.h>

#include "math/pointd.hpp"
#include "math/vector2d.hpp"

namespace rl::io
{
    const static std::locale locale = std::locale("en_US.UTF-8");

    template <typename T>
    constexpr std::string to_string(const vector2<T>& vec)
    {
        return fmt::format("[{}, {}]", vec.x, vec.y);
    }
}
