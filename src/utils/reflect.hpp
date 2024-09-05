#pragma once

#include <array>
#include <concepts>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace rl::inline reflect {
    template <typename T>
    consteval decltype(auto) demangled_typename() {
#if defined(__clang__)
        constexpr std::string_view pref{ "[T = " };
        constexpr std::string_view suff{ "]" };
        constexpr std::string_view func{ __PRETTY_FUNCTION__ };
#elif defined(__GNUC__)
        constexpr std::string_view pref{ "with T = " };
        constexpr std::string_view suff{ "; " };
        constexpr std::string_view func{ __PRETTY_FUNCTION__ };
#elif defined(_MSC_VER)
        constexpr std::string_view pref{ "rl::reflect::demangled_typename<" };
        constexpr std::string_view suff{ ">(void)" };
        constexpr std::string_view func{ __FUNCSIG__ };
#endif
        constexpr auto start{ func.find(pref) + pref.size() };
        constexpr auto end{ func.find(suff) };
        constexpr auto size{ end - start };
        return func.substr(start, size);
    }

#if defined(__GNUC__)
    template <typename T>
    concept aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

    struct member_info {
        std::string name{};
        std::string type{};
        std::string value{};
    };

    namespace detail {
        constexpr static std::size_t max_depth{ 128 };
        constexpr static std::size_t max_depth_overflow{ std::size_t(-1) };

        struct name_skip_config {
            constexpr std::string_view apply(std::string_view sv) {
                sv.remove_prefix(std::min(prefix_size, sv.size()));
                sv.remove_suffix(std::min(suffix_size, sv.size()));
                if (delimiter.empty())
                    return sv;

                auto found = sv.find(delimiter);
                if (found != std::string_view::npos)
                    sv.remove_prefix(found + delimiter.size());

                return sv.substr(0, sv.find(')'));
            }

            std::size_t prefix_size{};
            std::size_t suffix_size{};
            std::string_view delimiter{};
        };

        struct any_type {
            template <typename T>
            constexpr operator T() const;
        };

        template <typename T>
        struct wrapper {
            const T value;
        };

        template <typename T>
        const wrapper<T> helper;

        template <typename T>
        constexpr const T& fake_object() noexcept {
            return helper<T>.value;
        }

        template <typename T, typename... Ts>
        auto test_is_braces_constructible(std::size_t)
            -> decltype(void(T{ std::declval<Ts>()... }), std::true_type{});

        template <typename T, typename... Ts>
        auto test_is_braces_constructible(...)
            -> std::false_type;

        template <std::size_t N>
        struct tie_as_tuple_t;

        template <>
        struct tie_as_tuple_t<1> {
            template <typename T>
            constexpr static auto as_tuple(T&& s) {
                auto&& [e0] = std::forward<T>(s);
                return std::tie(e0);
            }
        };

        template <>
        struct tie_as_tuple_t<2> {
            template <typename T>
            constexpr static auto as_tuple(T&& s) {
                auto&& [e0, e1] = std::forward<T>(s);
                return std::tie(e0, e1);
            }
        };

        template <>
        struct tie_as_tuple_t<3> {
            template <typename T>
            constexpr static auto as_tuple(T&& s) {
                auto&& [e0, e1, e3] = std::forward<T>(s);
                return std::tie(e0, e1, e3);
            }
        };

        template <std::size_t N, typename S>
        constexpr auto tie_as_tuple(S&& s) {
            return tie_as_tuple_t<N>::as_tuple(std::forward<S>(s));
        }

        template <std::size_t N, typename T, typename... Ts>
        struct count_aggregate_members : std::conditional_t<
                                             (N - 1 == max_depth)
                                                 ? false
                                                 : decltype(test_is_braces_constructible<T, Ts...>(0))::value,
                                             count_aggregate_members<
                                                 N + 1, T,
                                                 any_type,
                                                 Ts...>,
                                             std::integral_constant<
                                                 std::size_t,
                                                 N - 1 == max_depth
                                                     ? max_depth_overflow
                                                     : N>> {
        };

        template <aggregate T>
        using count_aggregate_members_t =
            count_aggregate_members<0, T, detail::any_type>;

        template <aggregate T>
        constexpr auto member_count_v{ count_aggregate_members_t<T>::value };

        template <auto ptr>
        constexpr auto get_member_name() noexcept {
            constexpr static std::string_view sv{ __PRETTY_FUNCTION__ };
            auto&& config{ name_skip_config{ 145, 1, "::" } };
            return config.apply(sv);
        }

        template <aggregate T, std::size_t... I>
        constexpr auto get_member_names_impl(std::index_sequence<I...>) {
            return std::array<std::string_view, sizeof...(I)>{
                get_member_name<std::addressof(std::get<I>(
                    tie_as_tuple<sizeof...(I)>(fake_object<T>())))>()...
            };
        }

        template <aggregate T>
        constexpr auto get_member_names() {
            constexpr auto count{ member_count_v<T> };
            return get_member_names_impl<T>(std::make_index_sequence<count>{});
        }

        template <aggregate T, std::size_t... I>
        constexpr auto get_member_info(const T& s, std::index_sequence<I...>) {
            constexpr auto member_names = get_member_names<T>();
            const auto members{ tie_as_tuple<sizeof...(I)>(s) };
            return std::vector<member_info>{
                member_info(
                    std::string{ member_names[I] },
                    std::string{ demangled_typename<decltype(std::get<I>(members))>() },
                    std::to_string(std::get<I>(members)))...
            };
        }
    }

    template <aggregate T>
    constexpr auto get_member_info(const T& s) {
        constexpr auto count{ detail::member_count_v<T> };
        return detail::get_member_info(s, std::make_index_sequence<count>{});
    }

    template <aggregate auto S>
    struct aggregate_traits {
        constexpr static void print() {
            std::println("  {} {{", demangled_typename<decltype(S)>());
            for (auto&& [name, type, value] : members)
                std::println("      [ {:<13} ] {:6} => {}", type, name, value);
            std::println("  }}");
        }

        constexpr static std::string_view type_name{ demangled_typename<decltype(S)>() };
        static inline const std::vector<member_info> members{ get_member_info(S) };
    };
}

namespace rl::test {
    struct aggregate_type {
        double dbl_val{};
        int int_val{};
        float flt_val{};
    };

    constexpr static void compile_time_test() {
        std::println("\nSTATIC DURATION INSTANCE:");
        constexpr static aggregate_type consteval_test{ 1.23, 4, 5.6f };
        reflect::aggregate_traits<consteval_test>::print();
    }

    static void runtime_test() {
        const aggregate_type runtime_test{ 6.9, 420, 6.9f };
        const auto member_info{ reflect::get_member_info(runtime_test) };

        std::println("\nRUNTIME INSTANCE:");
        std::println("  {} {{", reflect::demangled_typename<decltype(runtime_test)>());
        for (auto&& [name, type, value] : member_info)
            std::println("      {}: {} = {}", name, type, value);
        std::println("  }}");
    }

    int run_reflection_tests() {
        test::compile_time_test();
        test::runtime_test();
    }
#endif
}
