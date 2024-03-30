#pragma once

#include <array>
#include <type_traits>
#include <utility>
#include <vector>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/vector2d.hpp"
#include "utils/concepts.hpp"
#include "utils/conversions.hpp"
#include "utils/math.hpp"
#include "utils/numeric.hpp"
#include "utils/properties.hpp"

namespace rl::ds {
#pragma pack(4)

    template <rl::numeric T>
    class rect
    {
    public:
        constexpr ~rect() = default;

        // Default construct a 'null' rect
        constexpr rect()
            : pt{ point<T>::null() }
            , size{ dims<T>::null() }
        {
        }

        // copy construct rect<T> from 'other' l-value rect<T>
        constexpr rect(const rect<T>& other)
            : pt{ other.pt }
            , size{ other.size }
        {
        }

        // copy construct rect<floating_point>
        // from l-value rect<integer>
        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr rect(const rect<I>& other)
            : pt{ static_cast<T>(other.pt.x), static_cast<T>(other.pt.y) }
            , size{ static_cast<T>(other.size.width), static_cast<T>(other.size.height) }
        {
        }

        // construct rect<T> from other r-value rect<T>
        constexpr rect(rect<T>&& other) noexcept
            : pt{ std::move(other.pt) }
            , size{ std::move(other.size) }
        {
        }

        // construct rect<T> from (top left point) x and y, and width and height
        constexpr rect(const T x, const T y, const T width, const T height)
            : pt{ x, y }
            , size{ width, height }
        {
        }

        // construct rect from r-value (top left) point<T> and dims<T> size
        constexpr rect(point<T>&& pnt, dims<T>&& dims)
            : pt{ pnt }
            , size{ dims }
        {
        }

        template <rl::integer I>
            requires rl::floating_point<T>
        constexpr rect(const point<T>& pnt, dims<I>&& dims)
            : pt{ pnt }
            , size{ static_cast<T>(dims.width), static_cast<T>(dims.height) }
        {
        }

        // construct rect from l-value (top left) point<T> and dims<T> size
        constexpr rect(const point<T>& pnt, const dims<T>& dims)
            : pt{ pnt }
            , size{ dims }
        {
        }

        // 'null' representation of a rect<T>
        // Returns:
        //   rect<T>{
        //     point<T>{
        //       .x = std::numeric_limits<T>::max()
        //       .y = std::numeric_limits<T>::max() },
        //     size<T>{
        //       .width  = 0
        //       .height = 0 } }
        //
        //   Initialized as: rect<T>{ point<T>::null(), dims<T>::null() }
        constexpr static rect<T> null()
        {
            return rect<T>{
                point<T>::null(),
                dims<T>::null(),
            };
        }

        // 'zero' representation of a rect<T>
        // Returns:
        //   rect<T>{
        //     point<T>{
        //       .x = 0
        //       .y = 0 },
        //     size<T>{
        //       .width  = 0
        //       .height = 0 } }
        //
        //   Initialized as: rect<T>{ point<T>::zero(), dims<T>::zero() }
        constexpr static rect<T> zero()
        {
            return rect<T>{
                point<T>::zero(),
                dims<T>::zero(),
            };
        }

        // Returns the area of the rect<T>
        [[nodiscard]]
        constexpr T area() const
        {
            return size.area();
        }

        // Checks if the rectangle has no area
        [[nodiscard]]
        constexpr bool is_empty() const
            requires rl::floating_point<T>
        {
            constexpr T epsilon{ std::numeric_limits<T>::epsilon() };
            return std::abs(this->area()) <= epsilon;
        }

        // Checks if the rectangle has no area
        [[nodiscard]]
        constexpr bool is_empty() const
            requires rl::integer<T>
        {
            return math::equal(this->area(), 0);
        }

        // Checks if the rectangle has a negative width or height
        [[nodiscard]]
        constexpr bool is_invalid() const
        {
            return this->size.height < T(0) || this->size.width < T(0);
        }

        // Checks if the rectangle has invalid coordinates and is empty
        [[nodiscard]]
        constexpr bool is_null() const
        {
            return this->is_empty() && pt->is_null();
        }

        // Generates an array of vertices representing the rect to be used in an OpenGL VBO
        constexpr auto triangles() -> std::array<point<T>, 6>
        {
            point<T> tl{ pt.x, pt.y + size.height };
            point<T> bl{ pt.x, pt.y };
            point<T> tr{ tl.x + size.width, tl.y };
            point<T> br{ tl.x + size.width, tl.y - size.height };

            return std::array{
                tr,  // triangle 1, top right
                br,  // triangle 1, top bottom right
                tl,  // triangle 1, top left
                br,  // triangle 2, bottom right
                bl,  // triangle 2, bottom left
                tl,  // triangle 2, top left
            };
        }

        // Generates a packed array of vertex/color pairs to be used in an OpenGL VBO
        //
        // Parameters:
        //     clr - The color to pack into the return array for this rect.
        constexpr auto triangles(const color<f32>& clr)
            -> std::array<std::pair<point<f32>, color<f32>>, 6>

        {
            point<T> tl{ pt.x, pt.y + size.height };
            point<T> bl{ pt.x, pt.y };
            point<T> tr{ tl.x + size.width, tl.y };
            point<T> br{ tl.x + size.width, tl.y - size.height };

            return std::array{
                std::pair{ tr, clr },  // triangle 1, TR | color
                std::pair{ br, clr },  // triangle 1, BR | color
                std::pair{ tl, clr },  // triangle 1, TL | color
                std::pair{ br, clr },  // triangle 2, BR | color
                std::pair{ bl, clr },  // triangle 2, BL | color
                std::pair{ tl, clr },  // triangle 2, TL | color
            };
        }

        // returns list of packed verices and indexes for each triangle
        //       for both triangles to be used as Element   constexpr inline Buffer     Objects
        [[nodiscard]]
        auto triangle_ebo() const
        {
            point<T> tl{ pt.x, pt.y + size.height };
            point<T> bl{ pt.x, pt.y };
            point<T> tr{ tl.x + size.width, tl.y };
            point<T> br{ tl.x + size.width, tl.y - size.height };

            std::vector<point<f32>> pts = { tr, br, bl, tl };
            std::vector<u32> idx = {
                0, 1, 3,  // triangle 1 point indices
                1, 2, 3   // triangle 2 point indices
            };

            return std::pair{
                pts,
                idx,
            };
        }

        // expand current rect to include rect other
        [[nodiscard]]
        constexpr rect<T> expanded(const rect<T>& other) const
        {
            const T min_x{ math::min(this->pt.x, other.pt.x) };
            const T max_x{ math::max((this->pt.x + this->size.width),
                                     (other.pt.x + other.size.width)) };

            const T min_y{ math::min(this->pt.y, other.pt.y) };
            const T max_y{ math::max((this->pt.y + this->size.height),
                                     (other.pt.y + other.size.height)) };

            return rect<T>{
                point{
                    min_x,
                    min_y,
                },
                dims{
                    max_x - min_x,
                    max_y - min_y,
                },
            };
        }

        // // expand current rect by some margin on all sides
        // constexpr rect<T>& expand(const margin<T>& margin)
        // {
        //     this->pt.x -= margin.left;
        //     this->pt.y += margin.bottom;
        //     this->size.width += (margin.left + margin.right);
        //     this->size.height += (margin.top + margin.bottom);
        //     return *this;
        // }

        // expand current rect to include rect other
        constexpr rect<T>& expand(const rect<T>& other)
        {
            *this = std::move(this->expanded(other));
            return *this;
        }

        // Gets the center point of the rectangle
        [[nodiscard]]
        constexpr rect<T> expanded(T amount) const noexcept

        {
            return rect<T>{
                point{
                    this->pt.x - amount,
                    this->pt.y - amount,
                },
                dims{
                    this->size.width + (amount * 2),
                    this->size.height + (amount * 2),
                },
            };
        }

        [[nodiscard]]
        constexpr Side edge_overlap(T buffer_size, const point<T>& pnt) const noexcept
        {
            Side overlap{ Side::None };

            const rect bigger{ this->expanded(buffer_size) };
            const rect smaller{ this->expanded(-buffer_size) };

            if (bigger.contains(pnt) && !smaller.contains(pnt))
            {
                // pnt is below the top line of bigger and
                // pnt is above top line of smaller
                if (pnt.y < smaller.pt.y && pnt.y > bigger.pt.y)
                    overlap |= Side::Top;

                // pnt is below bottom line of smaller and
                // pnt is above bottom line of bigger
                if (pnt.y > (smaller.pt.y + smaller.size.height) &&
                    pnt.y < (bigger.pt.y + bigger.size.height))
                    overlap |= Side::Bottom;

                // pnt is to the right of bigger's left line and
                // pnt is to the left of smaller's left line
                if (pnt.x > bigger.pt.x && pnt.x < smaller.pt.x)
                    overlap |= Side::Left;

                // pnt is to the right of smaller's right line and
                // pnt in to the left of bigger's right line
                if (pnt.x > (smaller.pt.x + smaller.size.width) &&
                    pnt.x < (bigger.pt.x + bigger.size.width))
                    overlap |= Side::Right;
            }

            return overlap;
        }

        [[nodiscard]]
        constexpr rect<T> top(T expand) const noexcept
        {
            return rect<T>{
                { this->pt.x + expand, this->pt.y - expand },
                { this->size.width - expand * 2, expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> bottom(T expand) const noexcept
        {
            return rect<T>{
                { pt.x + expand, (pt.y + this->size.height) - expand },
                { this->size.width - expand * 2, expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> left(T expand) const noexcept
        {
            return rect<T>{
                { this->pt.x - expand, pt.y + expand },
                { expand * 2, this->size.height - expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> right(T expand) const noexcept
        {
            return rect<T>{
                { (this->pt.x + this->size.width) - expand, pt.y + expand },
                { expand * 2, this->size.height - expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> top_left(T expand) const noexcept
        {
            const point ret{ this->top_left() };
            return rect<T>{
                { ret - expand },
                { expand * 2, expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> top_right(T expand) const noexcept
        {
            const point ret{ this->top_right() };
            return rect<T>{
                { ret - expand },
                { expand * 2, expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> bot_left(T expand) const noexcept
        {
            const point ret{ this->bot_left() };
            return rect<T>{
                { ret - expand },
                { expand * 2, expand * 2 },
            };
        }

        [[nodiscard]]
        constexpr rect<T> bot_right(T expand) const noexcept
        {
            const point ret{ this->bot_right() };
            return rect<T>{
                { ret - expand },
                { expand * 2, expand * 2 },
            };
        }

        // Gets the top left point of the rectangle
        [[nodiscard]]
        constexpr point<T> top_left() const noexcept
        {
            return point<T>{
                pt.x,
                pt.y,
            };
        }

        // Gets the top right point of the rectangle
        [[nodiscard]]
        constexpr point<T> top_right() const noexcept
        {
            return point<T>{
                pt.x + size.width,
                pt.y,
            };
        }

        // Gets the bottom left point of the rectangle
        [[nodiscard]]
        constexpr point<T> bot_left() const noexcept
        {
            return point<T>{
                pt.x,
                pt.y + size.height,
            };
        }

        // Gets the bottom right point of the rectangle
        [[nodiscard]]
        constexpr point<T> bot_right() const noexcept
        {
            return point<T>{
                (pt.x + size.width),
                (pt.y + size.height),
            };
        }

        // Gets the center point of the rectangle
        [[nodiscard]]
        constexpr point<T> centroid() const noexcept
        {
            return point<T>{
                static_cast<T>(pt.x + (size.width / 2)),
                static_cast<T>(pt.y + (size.height / 2)),
            };
        }

        // Checks if the rect shares any space with pt
        [[nodiscard]]
        constexpr bool overlaps(const point<T>& pnt) const
        {
            return (pnt.x >= this->pt.x && pnt.x <= this->pt.x + this->size.width) &&
                   (pnt.y >= this->pt.y && pnt.y <= this->pt.y + this->size.height);
        }

        // Checks if the rects share any space with each other
        [[nodiscard]]
        constexpr bool overlaps(const rect<T>& other) const
        {
            // TODO: optimize
            return this->overlaps(other.top_left()) || this->overlaps(other.top_right()) ||
                   this->overlaps(other.bot_left()) || this->overlaps(other.bot_right());
        }

        // Checks if the rect fully contains the point
        [[nodiscard]]
        constexpr bool contains(const point<T>& point) const
        {
            return (point.x > this->pt.x && point.x < this->pt.x + this->size.width) &&
                   (point.y > this->pt.y && point.y < this->pt.y + this->size.height);
        }

        // Checks if the rect fully contains the other
        [[nodiscard]]
        constexpr bool contains(const rect<T>& other) const
        {
            return this->contains(other.top_left()) &&  //
                   this->contains(other.top_right());
        }

        // Checks if the rect fully contains the other
        [[nodiscard]]
        constexpr bool contained_by(const rect<T>& other) const
        {
            return other.contains(this->top_left()) &&  //
                   other.contains(this->top_right());
        }

        // Checks if the point perfeclty falls somewhere on the rect's bounds.
        [[nodiscard]]
        constexpr bool touches(const point<T>& pnt) const
        {
            return (pnt.x == this->pt.x && this->pt.y <= pnt.y &&
                    pnt.y <= this->pt.y + this->size.width) ||  //
                   (pnt.y == this->pt.y && this->pt.x <= pnt.x &&
                    pnt.x <= this->pt.x + this->size.height);
        }

        // Checks if the this rect externally touches the other rect
        [[nodiscard]]
        constexpr bool touches(const rect<T>& other) const
        {
            return this->overlaps(other) && not this->overlaps(other);
        }

        // Returns a quadrant of the rectangle
        [[nodiscard]]
        constexpr rect<T> quad(Quad quad) const
        {
            point<T> center{ this->centroid() };
            dims<T> quad_size{
                this->size.width / static_cast<T>(2),
                this->size.height / static_cast<T>(2),
            };

            switch (quad)
            {
                case Quad::TopLeft:
                    return rect<T>{
                        { this->pt.x, this->pt.y },
                        { quad_size },
                    };
                case Quad::BottomLeft:
                    return rect<T>{
                        { this->pt.x, center.y },
                        { quad_size },
                    };
                case Quad::TopRight:
                    return rect<T>{
                        { center.x, this->pt.y },
                        { quad_size },
                    };
                case Quad::BottomRight:
                    return rect<T>{
                        { center.x, center.y },
                        { quad_size },
                    };
                default:
                    assert_msg("invalid quadrant value");
                    return {};
            }
        }

        // returns an array of the 4 quadrants of this constexpr inline rectangle
        [[nodiscard]]
        std::array<rect<T>, 4> quads() const
        {
            return std::array{
                this->quad(Quad::TopLeft),
                this->quad(Quad::BottomLeft),
                this->quad(Quad::TopRight),
                this->quad(Quad::BottomRight),
            };
        }

        // Returns a new rect scaled by the ratio, expanded / shrunk from the centroid
        [[nodiscard]]
        constexpr rect<T> scaled(vector2<f32>&& ratio) const
        {
            dims<T> scaled_size{
                T(cast::to<f32>(this->size.width) * ratio.x),
                T(cast::to<f32>(this->size.height) * ratio.y),
            };
            return rect<T>{
                this->centroid() - (scaled_size / 2.0f),
                scaled_size,
            };
        }

        // returns two halves of a rectange,
        // split either horizontally or vertically
        [[nodiscard]]
        constexpr std::array<rect<T>, 2> split(Axis axis) const
        {
            switch (axis)
            {
                case Axis::Horizontal:
                {
                    // split the rect in half using a
                    // horizontal line as the slice point
                    dims<T> half_size{
                        this->size.width,
                        this->size.height / T(2),
                    };
                    return std::array{
                        rect<T>{
                            point<T>(this->pt),
                            dims<T>(half_size),
                        },
                        rect<T>{
                            point<T>(this->pt + T(0), this->size.height / T(2)),
                            dims<T>(half_size),
                        },
                    };
                }
                case Axis::Vertical:
                {
                    // split the rect in half using a
                    // vertical line as the slice point
                    dims<T> half_size{
                        this->size.width / T(2),
                        this->size.height,
                    };
                    return std::array{
                        rect<T>{ this->pt, half_size },
                        rect<T>{
                            {
                                this->pt + (this->size.height / T(2)),
                                T(0),
                            },
                            half_size,
                        },
                    };
                }
                default:
                    assert_msg("invalid axis value");
                    return {};
            }
        }

    public:
        // Copy assignment
        constexpr bool operator==(const rect<T>& other) const
        {
            return (this->pt == other.pt) &&  //
                   (this->size == other.size);
        }

        // Copy assignment
        constexpr rect<T>& operator=(const rect<T>& other)
        {
            this->pt = std::forward<point<T>>(other.pt);
            this->size = std::forward<dims<T>>(other.size);
            return *this;
        }

        // Move assignment
        constexpr rect<T>& operator=(rect<T>&& other) noexcept
        {
            this->pt = std::move(other.pt);
            this->size = std::move(other.size);
            return *this;
        }

        // Moves the rectangle by the magnitude of the vector
        constexpr const rect<T>& operator+=(const vector2<T>& vec)
        {
            this->pt += vec;
            return *this;
        }

        // Returns a rectangle that's been moved by the magnitude of the vector
        constexpr rect<T> operator+(const vector2<T>& vec) const
        {
            rect<T> ret{ *this };
            ret += vec;
            return ret;
        }

    public:
        // position of the rect's top left point
        point<T> pt{};
        // 2D size of the rect, relative to pt
        // size assumed to always be positive
        dims<T> size{};
    };

    template <rl::numeric T>
    constexpr auto format_as(const rect<T>& r)
    {
        return fmt::format("[{} | {}]", r.pt, r.size);
    }

#pragma pack()
}
