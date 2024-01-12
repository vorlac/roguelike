#pragma once

#include <array>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/triangle.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_rect.h>
SDL_C_LIB_END

namespace rl::ds {
#pragma pack(4)

    enum Side : i8_fast {
        Top = 1 << 0,
        Bottom = 1 << 1,
        Left = 1 << 2,
        Right = 1 << 3,
    };

    enum Quad : i8_fast {
        TopLeft = (Side::Top | Side::Left),
        BottomLeft = (Side::Bottom | Side::Left),
        TopRight = (Side::Top | Side::Right),
        BottomRight = (Side::Bottom | Side::Right),
    };

    enum Axis : i8_fast {
        Horizontal = 1,  // x axis
        Vertical = 2,    // y axis
    };

    template <rl::numeric T>
    class rect
    {
    public:
        // Default construct a 'null' rect
        explicit constexpr inline rect()
            : pt{ ds::point<T>::null() }
            , size{ ds::dims<T>::null() }
        {
        }

        // copy construct rect<T> from 'other' l-value rect<T>
        constexpr inline rect(const rect<T>& other)
            : pt{ other.pt }
            , size{ other.size }
        {
        }

        // copy construct rect<floating_point>
        // from l-value rect<integer>
        template <rl::integer I>
        constexpr inline rect(const rect<I>& other)
            requires rl::floating_point<T>
            : pt(T(other.pt.x), T(other.pt.y))
            , size(T(other.size.width), T(other.size.height))
        {
        }

        // construct rect<T> from other r-value rect<T>
        constexpr inline rect(rect<T>&& other) noexcept
            : pt{ std::forward<point<T>>(other.pt) }
            , size{ std::forward<dims<T>>(other.size) }
        {
        }

        // construct rect<T> from (top left point) x and y, and width and height
        constexpr inline rect(const T x, const T y, const T width, const T height)
            : pt{ x, y }
            , size{ width, height }
        {
        }

        // construct rect from r-value (top left) point<T> and dims<T> size
        constexpr inline rect(point<T>&& pnt, dims<T>&& dims)
            : pt{ pnt }
            , size{ dims }
        {
        }

        // construct rect from l-value (top left) point<T> and dims<T> size
        constexpr inline rect(const point<T>& pnt, const dims<T>& dims)
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
        constexpr static inline rect<T> null()
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
        constexpr static inline rect<T> zero()
        {
            return rect<T>{
                point<T>::zero(),
                dims<T>::zero(),
            };
        }

        // construct rect<f32> from SDL_FRect&&
        constexpr rect(SDL3::SDL_FRect&& other)
            requires std::same_as<T, f32>
            : pt{ other.x, other.y }
            , size{ other.w, other.h }
        {
        }

        // cast rect<f32> to SDL_FRect
        constexpr operator SDL3::SDL_FRect()
            requires std::same_as<T, f32>
        {
            return {
                pt.x,
                pt.y,
                size.width,
                size.height,
            };
        }

        // Implicit conversion from `const rect<T>` to `const SDL_FRect*`
        // Returns:
        //     `nullptr` if the `rect<T>` is equivalent to `rect<T>::null()`,
        //     otherwise will return `const SDL_FRect*` equivalent to `this`
        constexpr operator const SDL3::SDL_FRect*() const
            requires std::same_as<T, f32>
        {
            if (this->is_null())
                return nullptr;
            return reinterpret_cast<const SDL3::SDL_FRect*>(this);
        }

        // Construct `rect<i32>` from r-value `SDL_Rect`
        constexpr rect(SDL3::SDL_Rect&& other)
            requires std::same_as<T, i32>
            : pt{ other.x, other.y }
            , size{ other.w, other.h }
        {
        }

        // Implicit conversion from `const rect<i32>&` to `SDL_Rect`
        constexpr operator const SDL3::SDL_Rect&() const&
            requires std::same_as<T, i32>
        {
            return *reinterpret_cast<const SDL3::SDL_Rect*>(this);
        }

        // Implicit conversion from `rect<i32>` to `SDL_Rect*`
        constexpr operator SDL3::SDL_Rect*()
            requires std::same_as<T, i32>
        {
            if (this->is_null())
                return nullptr;
            return reinterpret_cast<SDL3::SDL_Rect*>(this);
        }

        // Implicit conversion from `const rect<i32>` to `const SDL_Rect*`
        constexpr operator const SDL3::SDL_Rect*() const&
            requires std::same_as<T, i32>
        {
            if (this->is_null())
                return nullptr;
            return std::bit_cast<const SDL3::SDL_Rect*>(this);
        }

        // Returns the area of the rect<T>
        constexpr inline T area() const
        {
            return size.area();
        }

        // Checks if the rectangle has no area
        constexpr inline bool is_empty() const
            requires rl::floating_point<T>
        {
            constexpr T epsilon{ std::numeric_limits<T>::epsilon() };
            return std::abs(this->area()) <= epsilon;
        }

        // Checks if the rectangle has no area
        constexpr inline bool is_empty() const
            requires rl::integer<T>
        {
            return this->area() == T(0);
        }

        // Checks if the rectangle has a negative width or height
        constexpr inline bool is_invalid() const
        {
            return this->size.height < T(0) || this->width < T(0);
        }

        // Checks if the rectangle has invalid coordinates and is empty
        constexpr inline bool is_null() const
        {
            return this->is_empty() && pt == vector2<T>::null();
        }

        // Generates an array of vertices representing the rect to be used in an OpenGL VBO
        constexpr inline auto triangles() -> std::array<ds::point<T>, 6>
        {
            ds::point<T> tl{ pt.x, pt.y + size.height };
            ds::point<T> bl{ pt.x, pt.y };
            ds::point<T> tr{ tl.x + size.width, tl.y };
            ds::point<T> br{ tl.x + size.width, tl.y - size.height };

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
        constexpr inline auto triangles(const ds::color<f32>& clr)
            -> std::array<std::pair<ds::point<f32>, ds::color<f32>>, 6>

        {
            ds::point<T> tl{ pt.x, pt.y + size.height };
            ds::point<T> bl{ pt.x, pt.y };
            ds::point<T> tr{ tl.x + size.width, tl.y };
            ds::point<T> br{ tl.x + size.width, tl.y - size.height };

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
        auto triangle_ebo() const
        {
            ds::point<T> tl{ pt.x, pt.y + size.height };
            ds::point<T> bl{ pt.x, pt.y };
            ds::point<T> tr{ tl.x + size.width, tl.y };
            ds::point<T> br{ tl.x + size.width, tl.y - size.height };

            std::vector<ds::point<f32>> pts = { tr, br, bl, tl };
            std::vector<u32> idx = {
                0, 1, 3,  // triangle 1 point indices
                1, 2, 3   // triangle 2 point indices
            };

            return std::pair{
                pts,
                idx,
            };
        }

        // Gets the top left point of the rectangle
        constexpr inline point<T> top_left() const
        {
            return point<T>{
                pt.x,
                pt.y,
            };
        }

        // Gets the top right point of the rectangle
        constexpr inline point<T> top_right() const
        {
            return point<T>{
                pt.x + size.width,
                pt.y,
            };
        }

        // Gets the bottom left point of the rectangle
        constexpr inline point<T> bot_left() const
        {
            return point<T>{
                pt.x,
                pt.y + size.height,
            };
        }

        // Gets the bottom right point of the rectangle
        constexpr inline point<T> bot_right() const
        {
            return point<T>{
                pt.x + size.width,
                pt.y + size.height,
            };
        }

        // Gets the center point of the rectangle
        constexpr inline point<T> centroid() const
        {
            return point<T>{
                pt.x + (size.width / static_cast<T>(2)),
                pt.y + (size.height / static_cast<T>(2)),
            };
        }

        // Gets the center point of the rectangle
        constexpr inline rect<T> inflated(const T amount) const
        {
            return rect<T>{
                pt - (amount / static_cast<T>(2)),
                size + amount,
            };
        }

        // Checks if the rect shares any space with pt
        constexpr inline bool overlaps(const ds::point<T>& point) const
        {
            return (point.x >= this->pt.x && point.x <= this->pt.x + this->size.width) &&
                   (point.y >= this->pt.y && point.y <= this->pt.y + this->size.height);
        }

        // Checks if the rects share any space with each other
        constexpr inline bool overlaps(const ds::rect<T>& other) const
        {
            // TODO: optimize
            return this->overlaps(other.top_left()) || this->overlaps(other.top_right()) ||
                   this->overlaps(other.bot_left()) || this->overlaps(other.bot_right());
        }

        // Checks if the rect fully contains the point
        constexpr inline bool contains(const ds::point<T>& point) const
        {
            return (point.x > this->pt.x && point.x < this->pt.x + this->size.width) &&
                   (point.y > this->pt.y && point.y < this->pt.y + this->size.height);
        }

        // Checks if the rect fully contains the other

        constexpr inline bool contains(const ds::rect<T>& other) const
        {
            return this->contains(other.top_left()) &&  //
                   this->contains(other.top_right());
        }

        // Checks if the rect fully contains the other
        constexpr inline bool contained_by(const ds::rect<T>& other) const
        {
            return other.contains(this->top_left()) &&  //
                   other.contains(this->top_right());
        }

        // Checks if the point perfeclty falls somewhere on the rect's bounds.
        constexpr inline bool touches(const ds::point<T>& pnt) const
        {
            return (pnt.x == this->pt.x && this->pt.y <= pnt.y &&
                    pnt.y <= this->pt.y + this->size.width) ||  //
                   (pnt = this->pt.y && this->pt.x <= pnt.x &&
                          pnt.x <= this->pt.x + this->size.height);
        }

        // Checks if the this rect externally touches the other rect
        constexpr inline bool touches(const ds::rect<T>& other) const
        {
            return this->overlaps(other) && not this->intersects(other);
        }

        // Returns a quadrant of the rectangle
        constexpr inline rect<T> quad(Quad quad) const
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
            }
        }

        // returns an array of the 4 quadrants of this constexpr inline rectangle
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
        constexpr inline rect<T> scaled(ds::vector2<f32> ratio) const
        {
            ds::dims<T> scaled_size{
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
        constexpr inline std::array<rect<T>, 2> split(ds::Axis axis) const
        {
            if (axis == ds::Axis::Horizontal)
            {
                // split the rect in half using a
                // horizontal line as the slice point
                ds::dims<T> half_size{
                    this->size.width,
                    this->size.height / T(2),
                };
                return std::array{
                    ds::rect<T>{
                        ds::point<T>(this->pt),
                        ds::dims<T>(half_size),
                    },
                    ds::rect<T>{
                        ds::point<T>(this->pt + T(0), this->size.height / T(2)),
                        ds::dims<T>(half_size),
                    },
                };
            }
            else if (axis == ds::Axis::Vertical)
            {
                // split the rect in half using a
                // vertical line as the slice point
                ds::dims<T> half_size{
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
        }

    public:
        // Copy assignment
        constexpr rect<T>& operator=(const rect<T>& other)
        {
            std::memcpy(this, &other, sizeof(*this));
            return *this;
        }

        // Move assignment
        constexpr rect<T>& operator=(rect<T>&& other) noexcept
        {
            std::memcpy(this, &other, sizeof(*this));
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

        // position of the rect's top left point
        point<T> pt{
            T(0),  // x
            T(0),  // y
        };

        // 2D size of the rect, relative to pt
        dims<T> size{
            T(0),  // width
            T(0),  // height
        };
    };

    template <rl::numeric T>
    constexpr auto format_as(const rect<T>& r)
    {
        return fmt::format("[{} | {}]", r.pt, r.size);
    }

#pragma pack()
}
