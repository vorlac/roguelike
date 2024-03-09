#pragma once

#include <memory>
#include <utility>

#include "ds/point.hpp"
#include "utils/concepts.hpp"
#include "utils/memory.hpp"
#include "utils/numeric.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct line
    {
        constexpr line() = default;
        constexpr ~line() = default;

        constexpr line(const ds::point<T>& s, const ds::point<T>& e)
            : start{ std::forward<decltype(s)>(s) }
            , end{ std::forward<decltype(e)>(e) }
        {
        }

        constexpr line(ds::point<T>&& s, ds::point<T>&& e)
            : start{ std::move(s) }
            , end{ std::move(e) }
        {
        }

        ds::point<T> start{};
        ds::point<T> end{};
    };

#pragma pack()
}
