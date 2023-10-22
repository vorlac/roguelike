#pragma once

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"

namespace rl::ds
{
    enum class Quad
    {
        TopLeft = 0,
        BottomLeft = 1,
        TopRight = 2,
        BottomRight = 3,
    };

    template <typename T = float>
        requires Numeric<T>
    class rect
    {
    public:
        constexpr explicit rect(T x, T y, T width, T height)
            : m_pt{ x, y }
            , m_size{ width, height }
        {
        }

        constexpr explicit rect(point<T> pt, dimensions<T> size)
            : m_pt{ pt }
            , m_size{ size }
        {
        }

        constexpr explicit rect(const rect<T>& other)
        {
            this->operator=(std::forward<const rect<T>>(other));
        }

        constexpr explicit rect(rect<T>&& other)
        {
            this->operator=(other);
        }

        constexpr rect<T>& operator=(const rect<T>& other)
        {
            std::memcpy(this, &other, sizeof(*this));
            return *this;
        }

        constexpr operator raylib::Rectangle()
            requires std::same_as<T, float>
        {
            return *reinterpret_cast<raylib::Rectangle*>(this);
        }

        constexpr operator raylib::Rectangle()
            requires(!std::same_as<T, float>)
        {
            return raylib::Rectangle{
                .x = static_cast<float>(m_pt.x),
                .y = static_cast<float>(m_pt.y),
                .width = static_cast<float>(m_size.width),
                .height = static_cast<float>(m_size.height),
            };
        }

        constexpr inline T height() const
        {
            return m_size.height;
        }

        inline T width() const
        {
            return m_size.height;
        }

        constexpr inline void set_height(const T height)
        {
            m_size.height = height;
        }

        constexpr inline void set_width(const T width)
        {
            m_size.width = width;
        }

        constexpr inline T area() const
        {
            return m_size.width * m_size.height;
        }

        constexpr inline point<T> centroid() const
        {
            return {
                .x = m_pt.x + (m_size.width / static_cast<T>(2)),
                .y = m_pt.y + (m_size.height / static_cast<T>(2)),
            };
        }

        constexpr inline bool intersects(ds::point<T> pt) const
        {
            return pt.x > m_pt.x && pt.x < m_pt.x + m_size.width  //
                && pt.y > m_pt.y && pt.y < m_pt.y + m_size.height;
        }

        constexpr inline rect<T> quad(Quad quad) const
        {
            const point<T> center{ this->centroid() };
            const dimensions<T> quad_size{
                m_size.width / static_cast<T>(2),
                m_size.height / static_cast<T>(2),
            };

            switch (quad)
            {
                case Quad::TopLeft:
                    return {
                        { m_pt.x, m_pt.y },
                        { quad_size },
                    };
                case Quad::BottomLeft:
                    return {
                        { m_pt.x, center.y },
                        { quad_size },
                    };
                case Quad::TopRight:
                    return {
                        { center.x, m_pt.y },
                        { quad_size },
                    };
                case Quad::BottomRight:
                    return {
                        { center.x, center.y },
                        { quad_size },
                    };
            }
        }

        constexpr inline rect<T> quads() const
        {
            return std::array<rect<T>, 4>{
                this->quad(Quad::TopLeft),
                this->quad(Quad::BottomLeft),
                this->quad(Quad::TopRight),
                this->quad(Quad::BottomRight),
            };
        }

    private:
        // position of the rect's top left point
        point<T> m_pt{ static_cast<T>(0), static_cast<T>(0) };
        // dimensions of the rect, relative to m_pt
        dimensions<T> m_size{ .width = 0, .height = 0 };
    };
}
