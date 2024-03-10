#pragma once

#include <memory>
#include <type_traits>

namespace rl {
    template <typename T>
    consteval auto fwd(T&& arg)
    {
        return std::forward<decltype(T)>(arg);
    }
}
