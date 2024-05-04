#pragma once

#include <array>
#include <concepts>
#include <initializer_list>
#include <memory>
#include <numbers>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace rl::ds::test {

    template <typename T, std::size_t N>
    class grid
    {
        using size_type = std::size_t;

    public:
        constexpr grid(std::initializer_list<std::array<T, N>>&& rows)
            : m_width{ N }
            , m_height{ N }
        {
            // m_cells = std::vector{ rows | std::ranges::views::as_const | std::ranges::views::as_const };
            for (auto&& row : std::move(rows))
                m_cells.append_range(std::move(row));
            debug_assert(m_cells.size() == N * N);
        }

        constexpr size_type rows() const
        {
            return m_height;
        }

        constexpr size_type columns() const
        {
            return m_width;
        }

        constexpr const T& operator[](const std::pair<size_type, size_type>& idx) const
        {
            auto&& [row, col] = idx;
            return m_cells[row * m_width + col];
        }

    private:
        size_type m_width{ 0 };
        size_type m_height{ 0 };
        std::vector<T> m_cells;
    };

    using row_t = std::array<const char*, 4>;
    grid(std::initializer_list<std::array<const char*, 4>>&&)
        -> grid<std::string_view, 4>;

    int test_grid()
    {
        static const grid cell_grid{ {
            { "A1", "B1", "C1", "D1" },
            { "A2", "B2", "C2", "D2" },
            { "A3", "B3", "C3", "D3" },
            { "A4", "B4", "C4", "D4" },
        } };

        using namespace std::literals;
        for (std::size_t row : std::views::iota(0u, cell_grid.rows()))
            for (std::size_t col : std::views::iota(0u, cell_grid.columns()))
                std::print("{}{}", col == 0 ? "\n" : " ", cell_grid[{ row, col }]);
    }

}
