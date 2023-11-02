#pragma once

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/numeric_types.hpp"

namespace rl::ds
{
    enum Side : i8_fast {
        Top    = 1 << 0,
        Bottom = 1 << 1,
        Left   = 1 << 2,
        Right  = 1 << 3,
    };

    enum Quad : i8_fast {
        TopLeft     = (Side::Top | Side::Left),
        BottomLeft  = (Side::Bottom | Side::Left),
        TopRight    = (Side::Top | Side::Right),
        BottomRight = (Side::Bottom | Side::Right),
    };

    enum Axis : i8_fast {
        Horizontal = 1,  // x axis
        Vertical   = 2,  // y axis
    };

    template <rl::numeric T>
    class rect
    {
    public:
        constexpr explicit rect(const T x, const T y, const T width, const T height)
            : m_pt{ x, y }
            , m_size{ width, height }
        {
        }

        constexpr explicit rect(point<T>&& pt, dimensions<T>&& size)
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

        constexpr operator raylib::Rectangle()
            requires std::same_as<T, f32>
        {
            return *reinterpret_cast<raylib::Rectangle*>(this);
        }

        constexpr operator raylib::Rectangle()
            requires(!std::same_as<T, f32>)
        {
            return raylib::Rectangle{
                .x      = cast::to<f32>(m_pt.x),
                .y      = cast::to<f32>(m_pt.y),
                .width  = cast::to<f32>(m_size.width),
                .height = cast::to<f32>(m_size.height),
            };
        }

        /**
         * @brief Get rectangle's height
         * */
        constexpr inline T height() const
        {
            return m_size.height;
        }

        /**
         * @brief Set rectangle's height
         * */
        constexpr inline void height(const T height)
        {
            m_size.height = height;
        }

        /**
         * @brief Get rectangle's width
         * */
        inline T width() const
        {
            return m_size.height;
        }

        /**
         * @brief Set rectangle's width
         * */
        constexpr inline void width(const T width)
        {
            m_size.width = width;
        }

        /**
         * @brief Returns the rectangle's area
         * */
        constexpr inline T area() const
        {
            return m_size.area();
        }

        /**
         * @brief Checks if the rectangle has no area
         * */
        constexpr inline bool is_empty() const
        {
            return this->area() == cast::to<T>(0);
        }

        /**
         * @brief Checks if the rectangle has invalid coordinates and is empty
         * */
        constexpr inline bool is_null() const
        {
            return this->is_empty() && m_pt == vector2<T>::zero();
        }

        /**
         * @brief Gets the top left point of the rectangle
         * */
        constexpr inline point<T> top_left() const
        {
            return {
                .x = m_pt.x,
                .y = m_pt.y,
            };
        }

        /**
         * @brief Gets the top right point of the rectangle
         * */
        constexpr inline point<T> top_right() const
        {
            return {
                .x = m_pt.x + m_size.width,
                .y = m_pt.y,
            };
        }

        /**
         * @brief Gets the bottom left point of the rectangle
         * */
        constexpr inline point<T> bot_left() const
        {
            return {
                .x = m_pt.x,
                .y = m_pt.y + m_size.height,
            };
        }

        /**
         * @brief Gets the bottom right point of the rectangle
         * */
        constexpr inline point<T> bot_right() const
        {
            return {
                .x = m_pt.x + m_size.width,
                .y = m_pt.y + m_size.height,
            };
        }

        /**
         * @brief Gets the center point of the rectangle
         * */
        constexpr inline point<T> centroid() const
        {
            return {
                .x = m_pt.x + (m_size.width / cast::to<T>(2)),
                .y = m_pt.y + (m_size.height / cast::to<T>(2)),
            };
        }

        // TODO: look up if intersects and overlaps are named backwards

        /**
         * @brief Checks if the rect shares any space with pt
         * */
        constexpr inline bool overlaps(const ds::point<T>& pt) const
        {
            return (pt.x >= m_pt.x && pt.x <= m_pt.x + m_size.width) &&
                   (pt.y >= m_pt.y && pt.y <= m_pt.y + m_size.height);
        }

        /**
         * @brief Checks if the rects share any space with each other
         * */
        constexpr inline bool overlaps(const ds::rect<T>& other) const
        {
            // TODO: optimize
            return this->overlaps(other.top_left()) || this->overlaps(other.top_right()) ||
                   this->overlaps(other.bot_left()) || this->overlaps(other.bot_right());
        }

        /**
         * @brief Checks if the rect intersects with the point beyond
         *        just a single pixel touch/overlap.
         * */
        constexpr inline bool intersects(const ds::point<T>& pt) const
        {
            return (pt.x > m_pt.x && pt.x < m_pt.x + m_size.width) &&
                   (pt.y > m_pt.y && pt.y < m_pt.y + m_size.height);
        }

        /**
         * @brief Checks if the rect intersects with the point beyond
         *        just a single pixel touch/overlap.
         * */
        constexpr inline bool intersects(const ds::rect<T>& other) const
        {
            return this->contains(other.top_left()) || this->contains(other.top_right()) ||
                   this->contains(other.bot_left()) || this->contains(other.bot_right());
        }

        /**
         * @brief Checks if the rect fully contains the point
         * */
        constexpr inline bool contains(const ds::point<T>& pt) const
        {
            return (m_pt.x < pt.x && pt.x > m_pt.x + m_size.width) &&
                   (m_pt.y < pt.y && pt.y > m_pt.y + m_size.height);
        }

        /**
         * @brief Checks if the rect fully contains the other
         * */
        constexpr inline bool contains(const ds::rect<T>& other) const
        {
            return this->contains(other.top_left()) &&  //
                   this->contains(other.top_right());
        }

        /**
         * @brief Checks if the rect fully contains the other
         * */
        constexpr inline bool contained_by(const ds::rect<T>& other) const
        {
            return other.contains(this->top_left()) &&  //
                   other.contains(this->top_right());
        }

        /*
         * @brief Checks if the point perfeclty falls somewhere on the rect's bounds
         * */
        constexpr inline bool touches(const ds::point<T>& pt) const
        {
            return (pt.x == m_pt.x && m_pt.y <= pt.y && pt.y <= m_pt.y + m_size.width) ||  //
                   (pt.y == m_pt.y && m_pt.x <= pt.x && pt.x <= m_pt.x + m_size.height);
        }

        /*
         * @brief Checks if the this rect externally touches the other rect
         * */
        constexpr inline bool touches(const ds::rect<T>& other) const
        {
            return this->overlaps(other) && not this->intersects(other);
        }

        /**
         * @brief Returns a quadrant of the rectangle
         * @param quad The quadrant to return
         * */
        constexpr inline rect<T> quad(Quad quad) const
        {
            constexpr point<T> center{ this->centroid() };
            const dimensions<T> quad_size{
                m_size.width / cast::to<T>(2),
                m_size.height / cast::to<T>(2),
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

        /**
         * returns an array of the 4 quadrants of this rectangle
         * */
        inline rect<T> quads() const
        {
            std::array<rect<T>, 4> quadrants{ 0 };
            quads[Quad::TopLeft]     = this->quad(Quad::TopLeft);
            quads[Quad::BottomLeft]  = this->quad(Quad::BottomLeft);
            quads[Quad::TopRight]    = this->quad(Quad::TopRight);
            quads[Quad::BottomRight] = this->quad(Quad::BottomRight);
            return quadrants;
        }

        /**
         * returns two halves of a rectange,
         * split either horizontally or vertically
         * */
        constexpr inline std::array<rect<T>, 2> split(ds::Axis axis) const
        {
            if (axis == ds::Axis::Horizontal)
            {
                // split the rect in half using a
                // horizontal line as the slice point
                ds::dimensions<T> size{
                    m_size.width,
                    m_size.height / cast::to<T>(2),
                };
                return std::array{
                    rect<T>{ m_pt, size },
                    rect<T>{
                        {
                            m_pt + cast::to<T>(0),
                            m_size.height / cast::to<T>(2),
                        },
                        size,
                    },
                };
            }
            else if (axis == ds::Axis::Vertical)
            {
                // split the rect in half using a
                // vertical line as the slice point
                ds::dimensions<T> size{
                    m_size.width / cast::to<T>(2),
                    m_size.height,
                };
                return std::array{
                    rect<T>{ m_pt, size },
                    rect<T>{
                        {
                            m_pt + (m_size.height / cast::to<T>(2)),
                            cast::to<T>(0),
                        },
                        size,
                    },
                };
            }
        }

    public:
        /**
         * @brief Copy assignment
         * */
        constexpr rect<T>& operator=(const rect<T>& other)

        {
            std::memcpy(this, &other, sizeof(*this));
            return *this;
        }

        /**
         * @brief Move assignment
         * */
        constexpr rect<T>& operator=(rect<T>&& other)
        {
            std::memcpy(this, &other, sizeof(*this));
            return *this;
        }

        /**
         * @brief Moves the rectangle by the magnitude of the vector
         * */
        constexpr const rect<T>& operator+=(const vector2<T>& vec)
        {
            m_pt += vec;
            return *this;
        }

        /**
         * @brief Returns a rectangle that's been moved by the magnitude of the vector
         * */
        constexpr rect<T> operator+(const vector2<T>& vec) const
        {
            rect<T> ret{ *this };
            ret += vec;
            return ret;
        }

    private:
        // position of the rect's top left point
        point<T> m_pt{
            cast::to<T>(0),  // x
            cast::to<T>(0),  // y
        };

        // 2D size of the rect, relative to m_pt
        dimensions<T> m_size{
            cast::to<T>(0),  // width
            cast::to<T>(0),  // height
        };
    };
}
