#pragma once
#include <array>
#include <concepts>
#include <initializer_list>
#include <numbers>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace rl::ds {

    template <typename T>
    class CellGrid
    {
    public:
        constexpr CellGrid(std::initializer_list<std::vector<T>>&& rows)
        {
            m_height = rows.size();
            for (auto&& row : rows)
            {
                if (m_width == 0)
                    m_width = row.size();
                for (auto&& val : row)
                    m_cells.emplace_back(val);
            }
        }

        [[nodiscard]]
        constexpr i32 rows() const
        {
            return static_cast<i32>(m_height);
        }

        [[nodiscard]]
        constexpr i32 columns() const
        {
            return static_cast<i32>(m_width);
        }

        void print()
        {
            // rl::ds::CellGrid grid{
            //     { "A1", "B1", "C1", "D1" },
            //     { "A2", "B2", "C2", "D2" },
            //     { "A3", "B3", "C3", "D3" },
            // };

            for (auto row : std::views::iota(0, this->rows()))
                for (auto col : std::views::iota(0, this->columns()))
                    fmt::print("{}{}", col == 0 ? '\n' : ' ', this->operator[]({ row, col }));
        }

        T& operator[](std::pair<size_t, size_t>&& idx)
        {
            auto&& [row, col] = idx;
            return m_cells[row * m_width + col];
        }

    private:
        std::size_t m_width{ 0 };
        std::size_t m_height{ 0 };
        std::vector<T> m_cells{};
    };

    CellGrid(std::initializer_list<std::vector<const char*>>) -> CellGrid<std::string>;
}
