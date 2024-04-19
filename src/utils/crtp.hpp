#pragma once

#include <concepts>
#include <iostream>
#include <memory>
#include <variant>
#include <vector>

namespace rl::crtp::example {
    template <typename... TVisitorFunction>
    struct variant_visitor : TVisitorFunction...
    {
        template <typename... TCallable>
        variant_visitor(TCallable&&... vc)
            : TVisitorFunction{ std::forward<TCallable>(vc) }...
        {
        }

        using TVisitorFunction::operator()...;
    };

    template <typename... TVisitorFunction>
    variant_visitor(TVisitorFunction...)
        -> variant_visitor<std::remove_reference_t<TVisitorFunction>...>;

    // clang-format off

    struct A {
        int a{ 0 };
    };
    struct B {
        int b{ 0 };
    };
    struct C {
        int c{ 0 };
    };
    struct D {
        int d{ 0 };
    };

    // clang-format on

    int test()
    {
        using variant_t = std::variant<A, B, C, D>;
        std::vector<variant_t> variants = {
            A{ .a = 123 },
            B{ .b = 234 },
            D{ .d = 987 },
            C{ .c = 457 },
            A{ .a = 647 },
            D{ .d = 666 },
        };

        for (auto&& var : variants) {
            std::visit(variant_visitor{
                           [](A& a_variant) {
                               std::cout << "A: " << a_variant.a << std::endl;
                           },
                           [](B& b_variant) {
                               std::cout << "B: " << b_variant.b << std::endl;
                           },
                           [](auto& other) {
                               using other_t = std::remove_cvref_t<decltype(other)>;
                               if constexpr (std::same_as<other_t, C>)
                                   std::cout << "C: " << other.c << std::endl;
                               if constexpr (std::same_as<other_t, D>)
                                   std::cout << "D: " << other.d << std::endl;
                           },
                       },
                       var);
        }
    }
}
