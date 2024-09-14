#include <concepts>
#include <cstdint>
#include <limits>
#include <print>
#include <ranges>
#include <type_traits>
#include <utility>

namespace rl::test::reverse {
    template <std::integral auto Num, std::integral auto Exp>
        requires(Exp >= 0)
    consteval auto pow() {
        if constexpr (Exp == 0)
            return 1;
        else if constexpr (Exp == 1 || Num == 0)
            return Num;
        else if constexpr (Exp > 1)
            return Num * pow<Num, Exp - 1>();
    }

    template <std::integral auto Num>
    struct digit_count {
        [[nodiscard]]
        static consteval uint8_t digits() {
            uint8_t ret{ 0 };
            auto num{ Num };
            while (num != 0 && ++ret)
                num /= 10;
            return ret;
        }

        using unsigned_t = std::make_unsigned_t<decltype(Num)>;
        constexpr static unsigned_t value{ digit_count::digits() };
    };

    template <std::integral auto Num>
    constexpr inline auto digit_count_v{ digit_count<Num>::value };

    template <std::integral auto Num, typename T, T... Index>
    consteval auto reverse_int(std::integer_sequence<T, Index...>) {
        using unsigned_t = std::make_unsigned_t<decltype(Num)>;
        return (
            static_cast<unsigned_t>(
                pow<10, (sizeof...(Index) - (Index + 1))>() *
                static_cast<unsigned_t>((Num / pow<10, Index>()) % 10)) +
            ...);
    }

    template <std::integral auto Num>
    consteval auto reverse() {
        constexpr auto max_val{ std::numeric_limits<decltype(Num)>::max() };
        constexpr auto digit_sequence{ std::make_integer_sequence<
            decltype(Num), digit_count_v<Num>>{} };
        return reverse_int<Num>(digit_sequence);
    }
}
