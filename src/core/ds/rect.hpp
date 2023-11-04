#pragma once

#include <memory>
#include <type_traits>

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
        constexpr rect(const T x, const T y, const T width, const T height)
            : pt{ x, y }
            , size{ width, height }
        {
        }

        constexpr rect(point<T> pt, dimensions<T> size)
            : pt{ pt }
            , size{ size }
        {
        }

        constexpr rect(const point<T>& pt, const dimensions<T>& size)
            : pt{ pt }
            , size{ size }
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

        /**
         * @brief Get rectangle's height
         * */
        constexpr inline T height() const
        {
            return size.height;
        }

        /**
         * @brief Set rectangle's height
         * */
        constexpr inline void height(const T height)
        {
            size.height = height;
        }

        /**
         * @brief Get rectangle's width
         * */
        constexpr inline T width() const
        {
            return size.height;
        }

        /**
         * @brief Set rectangle's width
         * */
        constexpr inline void width(const T width)
        {
            size.width = width;
        }

        /**
         * @brief Returns the rectangle's area
         * */
        constexpr inline T area() const
        {
            return size.area();
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
            return this->is_empty() && pt == vector2<T>::zero();
        }

        /**
         * @brief Gets the top left point of the rectangle
         * */
        constexpr inline point<T> top_left() const
        {
            return {
                pt.x,
                pt.y,
            };
        }

        /**
         * @brief Gets the top right point of the rectangle
         * */
        constexpr inline point<T> top_right() const
        {
            return {
                pt.x + size.width,
                pt.y,
            };
        }

        /**
         * @brief Gets the bottom left point of the rectangle
         * */
        constexpr inline point<T> bot_left() const
        {
            return {
                pt.x,
                pt.y + size.height,
            };
        }

        /**
         * @brief Gets the bottom right point of the rectangle
         * */
        constexpr inline point<T> bot_right() const
        {
            return {
                pt.x + size.width,
                pt.y + size.height,
            };
        }

        /**
         * @brief Gets the center point of the rectangle
         * */
        constexpr inline point<T> centroid() const
        {
            return {
                pt.x + (size.width / cast::to<T>(2)),
                pt.y + (size.height / cast::to<T>(2)),
            };
        }

        /**
         * @brief Checks if the rect shares any space with pt
         * */
        constexpr inline bool overlaps(const ds::point<T>& pt) const
        {
            return (pt.x >= this->pt.x && pt.x <= this->pt.x + this->size.width) &&
                   (pt.y >= this->pt.y && pt.y <= this->pt.y + this->size.height);
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
            return (pt.x > this->pt.x && pt.x < this->pt.x + this->size.width) &&
                   (pt.y > this->pt.y && pt.y < this->pt.y + this->size.height);
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
            return (this->pt.x < pt.x && pt.x > this->pt.x + this->size.width) &&
                   (this->pt.y < pt.y && pt.y > this->pt.y + this->size.height);
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
            return (pt.x == this->pt.x && this->pt.y <= pt.y &&
                    pt.y <= this->pt.y + this->size.width) ||  //
                   (pt = this->pt.y && this->pt.x <= pt.x && pt.x <= this->pt.x + this->size.height);
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
                this->size.width / cast::to<T>(2),
                this->size.height / cast::to<T>(2),
            };

            switch (quad)
            {
                case Quad::TopLeft:
                    return {
                        { this->pt.x, this->pt.y },
                        { quad_size },
                    };
                case Quad::BottomLeft:
                    return {
                        { this->pt.x, center.y },
                        { quad_size },
                    };
                case Quad::TopRight:
                    return {
                        { center.x, this->pt.y },
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
        constexpr inline rect<T> quads() const
        {
            std::array<rect<T>, 4> quadrants{
                this->quad(Quad::TopLeft),
                this->quad(Quad::BottomLeft),
                this->quad(Quad::TopRight),
                this->quad(Quad::BottomRight),
            };
            return quadrants;
        }

        /**
         * @ Returns a new rect scaled by the ratio, expanded/shrunk from the centroid
         * */
        constexpr inline rect<T> scaled(ds::vector2<f32> ratio) const
        {
            ds::dimensions<T> scaled_size{
                cast::to<T>(cast::to<f32>(this->size.width) * ratio.x),
                cast::to<T>(cast::to<f32>(this->size.height) * ratio.y),
            };
            return rect<T>{
                this->centroid() - (scaled_size / 2.0f),
                scaled_size,
            };
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
                    this->size.width,
                    this->size.height / cast::to<T>(2),
                };
                return std::array{
                    rect<T>{ this->pt, size },
                    rect<T>{
                        {
                            this->pt + cast::to<T>(0),
                            this->size.height / cast::to<T>(2),
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
                    this->size.width / cast::to<T>(2),
                    this->size.height,
                };
                return std::array{
                    rect<T>{ this->pt, size },
                    rect<T>{
                        {
                            this->pt + (this->size.height / cast::to<T>(2)),
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
            this->pt += vec;
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

        // position of the rect's top left point
        point<T> pt{
            cast::to<T>(0),  // x
            cast::to<T>(0),  // y
        };

        // 2D size of the rect, relative to pt
        dimensions<T> size{
            cast::to<T>(0),  // width
            cast::to<T>(0),  // height
        };
    };
}
