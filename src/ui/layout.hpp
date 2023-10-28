#pragma once

#include <cstdint>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

#include "core/utils/assert.hpp"
#include "core/utils/concepts.hpp"
#include "ui/control.hpp"
#include "ui/properties.hpp"

namespace rl::ui
{

    template <std::size_t Cols, std::size_t Rows>
        requires rl::PositiveInteger<Cols> && rl::PositiveInteger<Rows>
    class Layout : public Control
    {
    public:
        static constexpr auto layout_rows{ Rows };
        static constexpr auto layout_cols{ Cols };
        using layout_t = Layout<layout_cols, layout_rows>;
        using rows_t   = std::array<std::shared_ptr<Control>, layout_rows>;
        using cols_t   = std::array<rows_t, Cols>;

    public:
        constexpr Layout(LayoutMode mode = LayoutMode::Grid)
            : m_mode{ mode }
        {
        }

        constexpr ~Layout() override
        {
        }

        inline constexpr decltype(auto) operator[](std::size_t col)
        {
            return m_controls[col];
        }

        inline constexpr size_t columns() const
        {
            return Layout::layout_cols;
        }

        inline constexpr size_t rows() const
        {
            return Layout::layout_rows;
        }

    private:
        LayoutMode m_mode{ LayoutMode::Grid };
        std::array<                        // cols
            std::array<                    // rows
                std::shared_ptr<Control>,  // layout control
                layout_rows>,
            layout_cols>
            m_controls = { { { nullptr } } };
    };
}
