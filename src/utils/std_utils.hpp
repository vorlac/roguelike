#pragma once

#include <memory>
#include <string_view>

namespace rl {
    template <typename T>
    auto fwd(T&& arg) -> decltype(auto)
    {
        return std::forward<T>(arg);
    }

    template <typename T>
    constexpr std::string_view demangled_typename()
    {
#if defined(__clang__)
        constexpr std::string_view pref{ "[T = " };
        constexpr std::string_view suff{ "]" };
        constexpr std::string_view func{ __PRETTY_FUNCTION__ };
#elif defined(__GNUC__)
        constexpr std::string_view pref{ "with T = " };
        constexpr std::string_view suff{ "; " };
        constexpr std::string_view func{ __PRETTY_FUNCTION__ };
#elif defined(_MSC_VER)
        constexpr std::string_view pref{ "wolv::type::getTypeName<" };
        constexpr std::string_view suff{ ">(void)" };
        constexpr std::string_view func{ __FUNCSIG__ };
#endif

        constexpr auto start = func.find(pref) + pref.size();
        constexpr auto end = func.find(suff);
        constexpr auto size = end - start;
        return func.substr(start, size);
    }
}
