#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ds/point.hpp"
#include "utils/concepts.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    struct alignas(T) triangle
    {
        constexpr triangle()
            : a{ ds::point<T>{} }
            , b{ ds::point<T>{} }
            , c{ ds::point<T>{} }
        {
        }

        constexpr triangle(const ds::point<T>& _a, const ds::point<T>& _b, const ds::point<T>& _c)
            : a{ _a }
            , b{ _b }
            , c{ _c }
        {
        }

        constexpr inline T* data()
        {
            return &a;
        }

    private:
        ds::point<T> a{};
        ds::point<T> b{};
        ds::point<T> c{};
    };

    using trianglevec = std::vector<ds::triangle<f32>>;

#pragma pack()
}
