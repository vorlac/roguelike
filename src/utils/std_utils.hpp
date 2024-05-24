#pragma once

#include <string_view>

namespace rl {
    template <typename T>
    consteval decltype(auto) fwd(T&& arg)
    {
        return std::forward<decltype(T)>(arg);
    }

    template <typename T>
    consteval std::string_view demangled_typename()
    {
#if defined(__clang__)
        constexpr auto pref = std::string_view("[T = ");
        constexpr auto suff = "]";
        constexpr auto func = std::string_view(__PRETTY_FUNCTION__);
#elif defined(__GNUC__)
        constexpr auto pref = std::string_view("with T = ");
        constexpr auto suff = "; ";
        constexpr auto func = std::string_view(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
        constexpr auto pref = std::string_view("wolv::type::getTypeName<");
        constexpr auto suff = ">(void)";
        constexpr auto func = std::string_view(__FUNCSIG__);
#endif

        constexpr auto start = func.find(pref) + pref.size();
        constexpr auto end = func.find(suff);
        constexpr auto size = end - start;

        return func.substr(start, size);
    }
}
