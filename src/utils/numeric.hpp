#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace rl {
    using f32 = float;
    using f64 = double;

    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using i8_fast = int_fast8_t;
    using i16_fast = int_fast16_t;
    using i32_fast = int_fast32_t;
    using i64_fast = int_fast64_t;

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using u8_fast = uint_fast8_t;
    using u16_fast = uint_fast16_t;
    using u32_fast = uint_fast32_t;
    using u64_fast = uint_fast64_t;

}

namespace rl {

    template <typename TEnum>
        requires std::is_scoped_enum_v<TEnum>
    constexpr TEnum operator|(const TEnum lhs, const TEnum rhs)
    {
        return static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(lhs) |
                                  static_cast<std::underlying_type_t<TEnum>>(rhs));
    }

    template <typename TEnum>
        requires std::is_scoped_enum_v<TEnum>
    constexpr TEnum operator&(const TEnum lhs, const TEnum rhs)
    {
        return static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(lhs) &
                                  static_cast<std::underlying_type_t<TEnum>>(rhs));
    }

    template <typename TEnum, typename TUnderlying>
        requires std::is_scoped_enum_v<TEnum> &&
                 std::same_as<std::underlying_type_t<TEnum>, TUnderlying>
    constexpr bool operator==(const TUnderlying lhs, const TEnum rhs)
    {
        return lhs == static_cast<std::underlying_type_t<TEnum>>(rhs);
    }

    template <typename TEnum, typename TUnderlying>
        requires std::is_scoped_enum_v<TEnum> &&
                 std::same_as<std::underlying_type_t<TEnum>, TUnderlying>
    constexpr bool operator==(const TEnum lhs, const TUnderlying rhs)
    {
        return static_cast<std::underlying_type_t<TEnum>>(rhs) == lhs;
    }

    template <typename TEnum, typename TUnderlying>
        requires std::is_scoped_enum_v<TEnum> &&
                 std::same_as<std::underlying_type_t<TEnum>, TUnderlying>
    constexpr bool operator!=(const TUnderlying lhs, const TEnum rhs)
    {
        return !(lhs == rhs);
    }

    template <typename TEnum, typename TUnderlying>
        requires std::is_scoped_enum_v<TEnum> &&
                 std::same_as<std::underlying_type_t<TEnum>, TUnderlying>
    constexpr bool operator!=(const TEnum lhs, const TUnderlying rhs)
    {
        return !(lhs == rhs);
    }
};
