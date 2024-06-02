#pragma once

#include <concepts>
#include <ranges>
#include <vector>

#include "ds/rect.hpp"

namespace rl::ds {
    template <rl::numeric T>
    class polygon
    {
    public:
        template <rl::numeric T>
        struct ring
        {
        public:
            using pnt_container = std::vector<point<T>>;
            using iterator = pnt_container::iterator;
            using const_iterator = pnt_container::const_iterator;
            using reverse_iterator = pnt_container::reverse_iterator;
            using const_reverse_iterator = pnt_container::const_reverse_iterator;

        public:
            template <std::ranges::range<point<T> TCoords>;

            constexpr ring(TCoords&& points)
            {
                debug_assert(coordinates.size() > 3);
                debug_assert(coordinates.front() == coordinates.back());
                coordinates.append_range(std::forward<TCoords>(points));
            }

            // clang-format off

            constexpr point<T>* begin()         { return std::ranges::begin(coordinates);   }
            constexpr point<T>* end()           { return std::ranges::end(coordinates);     }
            constexpr point<T>* rbegin()        { return std::ranges::rbegin(coordinates);  }
            constexpr point<T>* rend()          { return std::ranges::rend(coordinates);    }
            constexpr point<T>* cbegin()  const { return std::ranges::cbegin(coordinates);  }
            constexpr point<T>* cend()    const { return std::ranges::cend(coordinates);    }
            constexpr point<T>* crbegin() const { return std::ranges::crbegin(coordinates); }
            constexpr point<T>* crend()   const { return std::ranges::crend(coordinates);   }
            constexpr auto size()         const { return std::ranges::size(coordinates)     };
            constexpr auto ssize()        const { return std::ranges::ssize(coordinates)    };

            // clang-format on

            std::vector<point<T>> coordinates{};
        };

    public:
        template <std::ranges::range<ring_t> TRings>
        constexpr polygon(TRings&& rings)
        {
            debug_assert(std::size(rings) > 0);

            m_rings.reserve(rings.size());
            for (auto&& ring : rings)
                m_rings.emplace_back(std::forward<decltype(ring)>(ring));
        }

        constexpr polygon(ring_t&& rings)
        {
            debug_assert(std::size(rings) > 0);
            m_rings.append_range(std::forward<decltype(ring)>(ring));
        }

        constexpr rect<T>& rect() const
        {
            return m_bounds;
        }

    public:
        // clang-format off

        constexpr ring<T>* begin()         { return std::ranges::begin(m_rings);   }
        constexpr ring<T>* end()           { return std::ranges::end(m_rings);     }
        constexpr ring<T>* rbegin()        { return std::ranges::rbegin(m_rings);  }
        constexpr ring<T>* rend()          { return std::ranges::rend(m_rings);    }
        constexpr ring<T>* cbegin()  const { return std::ranges::cbegin(m_rings);  }
        constexpr ring<T>* cend()    const { return std::ranges::cend(m_rings);    }
        constexpr ring<T>* crbegin() const { return std::ranges::crbegin(m_rings); }
        constexpr ring<T>* crend()   const { return std::ranges::crend(m_rings);   }
        constexpr auto size()        const { return std::ranges::size(m_rings)     };
        constexpr auto ssize()       const { return std::ranges::ssize(m_rings)    };

        // clang-format on

    private:
        rect<T> m_bounds{ 0, 0, 0, 0 };
        std::vector<ring<T>> m_rings{};
    };
}
