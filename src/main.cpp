// #include "core/application.hpp"
// #include "utils/options.hpp"

// clang-format off
#include <memory>
#include <type_traits>

#include <concepts>
#include <utility>

template<typename T, std::size_t size>
struct optimize_ref
{
    using t1 = typename std::remove_cvref_t<T>;
    using t2 = typename std::add_lvalue_reference_t<typename t1>;
    static constexpr bool test = sizeof(T) > size * 2;

    using type = std::conditional_t<test,
        std::add_lvalue_reference_t<t2>,
        std::remove_cvref_t<t1>>;
};

int main() {
    const optimize_ref<uint64_t, 1U>::type i1{'c'};
    const optimize_ref<char, 12U>::type i2{'c'};
}

// clang-format on
