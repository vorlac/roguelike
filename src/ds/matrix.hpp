#pragma once

#include <array>
#include <concepts>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

#include "utils/concepts.hpp"

template <typename TRef, std::size_t Size>
struct ref_array : std::span<TRef, Size> {
};

template <typename T, std::size_t N>
constexpr inline bool std::ranges::enable_borrowed_range<ref_array<T, N>> = true;

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T, u64 Rows, u64 Cols>
    struct matrix {
    public:
        consteval matrix() = default;

        consteval static matrix identity()
            requires(Rows == Cols)
        {
            matrix ret{};
            for (u32 r = 0; r < Rows; ++r)
                ret[{ r, r }] = 1;
            return ret;
        }

        template <typename TRow, typename... TOtherRows>
            requires(rl::all_of<std::array<T, Cols>, TRow, TOtherRows...> &&
                     1 + sizeof...(TOtherRows) == Rows)
        constexpr matrix(TRow&& first, TOtherRows&&... rest) {
            for (auto&& [r, row] : std::array{ first, rest... } | std::views::enumerate) {
                for (u64 c : std::views::iota(0U, Cols))
                    (*this)[{ r, c }] = row[c];
            }
        }

    public:
        [[nodiscard]]
        constexpr T x() const {
            debug_assert("not implemented");
            return m_rows[0ull];
        }

        [[nodiscard]]
        constexpr T y() const {
            debug_assert("not implemented");
            return m_rows[0ull];
        }

        [[nodiscard]]
        constexpr T z() const {
            debug_assert("not implemented");
            return m_rows[0ull];
        }

    public:
        // constexpr const T operator[](std::pair<u32, u32> idx) const
        //{
        //     auto&& [row, col] = idx;
        //     return m_rows[row][col];
        // }

        // [1, 2, 3]
        // [4, 5, 6]
        constexpr T& operator[](std::pair<u32, u32> idx) {
            auto&& [row, col] = idx;
            return m_rows[row][col];
        }

        constexpr matrix& operator+=(const matrix& other) {
            for (auto&& [row_idx, row] : m_rows | std::views::enumerate)
                for (auto&& [col_idx, col] : row | std::views::enumerate)
                    m_rows[row_idx][col_idx] += other[row_idx][col_idx];

            return *this;
        }

        constexpr matrix operator+(const matrix& other) {
            matrix ret{ *this };
            ret += other;
            return ret;
        }

        constexpr matrix& operator-=(const matrix& other) {
            for (auto&& [row_idx, row] : m_rows | std::views::enumerate)
                for (auto&& [col_idx, col] : row | std::views::enumerate)
                    m_rows[row_idx][col_idx] -= other[row_idx][col_idx];
            return *this;
        }

        constexpr matrix operator-(const matrix& other) {
            matrix ret{ *this };
            ret -= other;
            return ret;
        }

        constexpr matrix& operator*(const T scalar) {
            debug_assert("not implemented");
            return *this;
        }

        constexpr matrix& operator*=(const matrix& other) {
            for (auto row = 0; row < Rows; ++row) {
                for (auto col = 0; col < Cols; ++col) {
                    m_rows[{ row, col }] * other[{ col, row }];
                }
            }
            return *this;
        }

        constexpr matrix operator*(const matrix& other) {
            matrix ret{ *this };
            ret *= other;
            return ret;
        }

    private:
        consteval auto setup_references() {
            std::array<std::array<T, Cols>, Rows> ret{};
            for (auto col_idx : std::views::iota(0u, Cols)) {
                for (auto row_idx : std::views::iota(0u, Rows)) {
                    auto&& val = (*this)[{ row_idx, col_idx }];
                    ret[col_idx][row_idx] = std::ref(std::forward<decltype(val)>(val));
                }
            }
            return ret;
        }

        std::array<std::array<T, Cols>, Rows> m_rows{};
        std::array<std::array<T, Rows>, Cols> m_cols{ setup_references() };
    };

    template <typename TRow>
    using row_t = std::array<typename TRow::value_type, sizeof(TRow) / sizeof(TRow::value_type)>;

    template <typename TRow, typename... TOtherRows>
        requires(rl::all_of<row_t<TRow>, TRow, TOtherRows...>)
    matrix(TRow, TOtherRows...) -> matrix<typename TRow::value_type, 1 + sizeof...(TOtherRows),
                                          sizeof(TRow) / sizeof(TRow::value_type)>;
}
