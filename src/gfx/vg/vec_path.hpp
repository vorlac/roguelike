#pragma once
#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "ds/line.hpp"
#include "ds/rect.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    class path
    {
    public:
        // clang-format off

        enum class fill_mode {
            Winding,        /** Specifies that "inside" is computed by a non-zero sum of signed edge crossings */
            EvenOdd,        /** Specifies that "inside" is computed by an odd number of edge crossings */
            InverseWinding, /** Same as Winding, but draws outside of the path, rather than inside */
            InverseEvenOdd  /** Same as EvenOdd, but draws outside of the path, rather than inside */
        };

        enum class instruction {
            move    = 0x0000, // move from current location to new position
            line    = 1 << 0, // add line from current pos to new position
            bezier  = 1 << 2, // add bezier / quad from current pos to 
            close   = 1 << 3,
            winding = 1 << 4,
        };
        // clang-format on

        enum class op {
            difference,          //!< subtract the op path from the first path
            intersect,           //!< intersect the two paths
            merge,               //!< union (inclusive-or) the two paths
            exclusive_or,        //!< exclusive-or the two paths
            reverse_difference,  //!< subtract the first path from the op path
        };

        using step = std::pair<path::op, std::array<f32, 4>>;

    public:
        explicit path() = default;

        explicit path(std::vector<step> sequence)
            m_path_sequence{ std::move(sequence) }
        {
        }

        auto& move_to(ds::point<f32> pos)
        {
            m_path_sequence.emplace_back(op::move, { pos.x, pos.y });
            return *this;
        }

        auto& line_to(ds::point<f32> pos)
        {
            m_path_sequence.emplace_back(op::line, { pos.x, pos.y });
            return *this;
        }

        auto& add_rect(ds::rect<f32> rect)
        {
            m_path_sequence.emplace_back(op::rect, { pos.x, pos.y });
            return *this;
        }

    private:
        std::vector<step> m_path_sequence{};
    };
}
