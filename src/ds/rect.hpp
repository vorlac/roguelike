#pragma once

#include <memory>
#include <type_traits>

#include "core/numeric.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/triangle.hpp"
#include "ds/vector2d.hpp"
#include "sdl/defs.hpp"
#include "utils/concepts.hpp"

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
        explicit constexpr rect()
            : pt{ ds::point<T>::null() }
            , size{ ds::dims<T>::null() }
        {
        }

        constexpr rect(const rect<T>& other)
            : pt{ other.pt }
            , size{ other.size }
        {
        }

        template <rl::integer I>
        constexpr rect(const rect<I>& other)
            requires rl::floating_point<T>
            : pt(cast::to<T>(other.pt.x), cast::to<T>(other.pt.y))
            , size(cast::to<T>(other.size.width), cast::to<T>(other.size.height))
        {
        }

        constexpr rect(rect<T>&& other) noexcept
            : pt{ std::forward<point<T>>(other.pt) }
            , size{ std::forward<dims<T>>(other.size) }
        {
        }

        constexpr rect(const T x, const T y, const T width, const T height)
            : pt{ x, y }
            , size{ width, height }
        {
        }

        constexpr rect(point<T>&& pnt, dims<T>&& dims)
            : pt{ pnt }
            , size{ dims }
        {
        }

        constexpr rect(const point<T>& pnt, const dims<T>& dims)
            : pt{ pnt }
            , size{ dims }
        {
        }

        constexpr static inline rect<T> null()
        {
            return rect<T>{
                point<T>::null(),
                dims<T>::null(),
            };
        }

        constexpr static inline rect<T> zero()
        {
            return rect<T>{
                point<T>::zero(),
                dims<T>::zero(),
            };
        }

        /**
         * @brief construct rect<f32> from SDL_FRect&&
         * */
        constexpr rect(SDL3::SDL_FRect&& other)
            requires std::same_as<T, f32>
            : pt{ other.x, other.y }
            , size{ other.w, other.h }
        {
        }

        /**
         * @brief cast rect<f32> to SDL_FRect
         * */
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

        /**
         * @brief implicit cast from
         * const rect<f32>& to const SDL_FRect&
         * */
        constexpr operator const SDL3::SDL_FRect&() const&
            requires std::same_as<T, f32>
        {
            return *reinterpret_cast<const SDL3::SDL_FRect*>(this);
        }

        /**
         * @brief construct rect<f32> from SDL_FRect
         * */
        constexpr rect(const SDL3::SDL_FRect* other)
            requires std::same_as<T, f32>
            : pt{ other->x, other->y }
            , size{ other->w, other->h }
        {
        }

        constexpr operator const SDL3::SDL_FRect*() const
            requires std::same_as<T, f32>
        {
            if (this->is_null())
                return nullptr;
            return reinterpret_cast<const SDL3::SDL_FRect*>(this);
        }

        constexpr operator SDL3::SDL_FRect*()
            requires std::same_as<T, f32>
        {
            if (this->is_null())
                return nullptr;
            return reinterpret_cast<SDL3::SDL_FRect*>(this);
        }

        constexpr rect(SDL3::SDL_Rect&& other)
            requires std::same_as<T, i32>
            : pt{ other.x, other.y }
            , size{ other.w, other.h }
        {
        }

        constexpr rect(const SDL3::SDL_Rect& other)
            requires std::same_as<T, i32>
            : pt{ other.x, other.y }
            , size{ other.w, other.h }
        {
        }

        constexpr operator SDL3::SDL_Rect() const&
            requires std::same_as<T, i32>
        {
            return *reinterpret_cast<const SDL3::SDL_Rect*>(this);
        }

        constexpr operator SDL3::SDL_Rect*()
            requires std::same_as<T, i32>
        {
            if (this->is_null())
                return nullptr;
            return reinterpret_cast<SDL3::SDL_Rect*>(this);
        }

        constexpr operator const SDL3::SDL_Rect*() const&
            requires std::same_as<T, i32>
        {
            if (this->is_null())
                return nullptr;
            return reinterpret_cast<const SDL3::SDL_Rect*>(this);
        }

        constexpr inline T area() const
        {
            return size.area();
        }

        /**
         * @brief Checks if the rectangle has no area
         * */
        constexpr inline bool is_empty() const
            requires rl::floating_point<T>
        {
            constexpr T epsilon = std::numeric_limits<T>::epsilon();
            return std::abs(this->area()) <= epsilon;
        }

        /**
         * @brief Checks if the rectangle has no area
         * */
        constexpr inline bool is_empty() const
            requires rl::integer<T>
        {
            return this->area() == 0;
        }

        /**
         * @brief Checks if the rectangle has a negative width or height
         * */
        constexpr inline bool is_invalid() const
        {
            return this->size.height < cast::to<T>(0) ||  //
                   this->width < cast::to<T>(0);          //
        }

        /**
         * @brief Checks if the rectangle has invalid coordinates and is empty
         * */
        constexpr inline bool is_null() const
        {
            return this->is_empty() && pt == vector2<T>::null();
        }

        constexpr inline decltype(auto) triangles()
        {
            ds::point<T> tl{ pt.x, pt.y + size.height };
            ds::point<T> bl{ pt.x, pt.y };
            ds::point<T> tr{ tl.x + size.width, tl.y };
            ds::point<T> br{ tl.x + size.width, tl.y - size.height };

            return std::vector{
                ds::triangle{ bl, tl, br },
                ds::triangle{ tl, br, tl },
            };
        }

        /**
         * @brief returns list of packed verices and indexes for each triangle
         *        for both triangles to be used as Element Buffer Objects
         * */
        constexpr inline auto triangle_ebo() const
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

            return std::pair{ pts, idx };
        }

        /**
         * @brief Gets the top left point of the rectangle
         * */
        constexpr inline point<T> top_left() const
        {
            return { pt.x, pt.y };
        }

        /**
         * @brief Gets the top right point of the rectangle
         * */
        constexpr inline point<T> top_right() const
        {
            return { pt.x + size.width, pt.y };
        }

        /**
         * @brief Gets the bottom left point of the rectangle
         * */
        constexpr inline point<T> bot_left() const
        {
            return { pt.x, pt.y + size.height };
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
        constexpr inline bool overlaps(const ds::point<T>& pnt) const
        {
            return (pnt.x >= this->pt.x && pnt.x <= this->pt.x + this->size.width) &&
                   (pnt.y >= this->pt.y && pnt.y <= this->pt.y + this->size.height);
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
        constexpr inline bool intersects(const ds::point<T>& pnt) const
        {
            return (pnt.x > this->pt.x && pnt.x < this->pt.x + this->size.width) &&
                   (pnt.y > this->pt.y && pnt.y < this->pt.y + this->size.height);
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
        constexpr inline bool contains(const ds::point<T>& pnt) const
        {
            return (this->pt.x < pnt.x && pnt.x > this->pt.x + this->size.width) &&
                   (this->pt.y < pnt.y && pnt.y > this->pt.y + this->size.height);
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
        constexpr inline bool touches(const ds::point<T>& pnt) const
        {
            return (pnt.x == this->pt.x && this->pt.y <= pnt.y &&
                    pnt.y <= this->pt.y + this->size.width) ||  //
                   (pnt = this->pt.y && this->pt.x <= pnt.x &&
                          pnt.x <= this->pt.x + this->size.height);
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
            const dims<T> quad_size{
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
            ds::dims<T> scaled_size{
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
                ds::dims<T> half_size{
                    this->size.width,
                    this->size.height / cast::to<T>(2),
                };
                return std::array{
                    ds::rect<T>{
                        ds::point<T>(this->pt),
                        ds::dims<T>(half_size),
                    },
                    ds::rect<T>{
                        ds::point<T>(this->pt + cast::to<T>(0), this->size.height / cast::to<T>(2)),
                        ds::dims<T>(half_size),
                    },
                };
            }
            else if (axis == ds::Axis::Vertical)
            {
                // split the rect in half using a
                // vertical line as the slice point
                ds::dims<T> half_size{
                    this->size.width / cast::to<T>(2),
                    this->size.height,
                };
                return std::array{
                    rect<T>{ this->pt, half_size },
                    rect<T>{
                        {
                            this->pt + (this->size.height / cast::to<T>(2)),
                            cast::to<T>(0),
                        },
                        half_size,
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
        constexpr rect<T>& operator=(rect<T>&& other) noexcept
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
        dims<T> size{
            cast::to<T>(0),  // width
            cast::to<T>(0),  // height
        };
    };

    template <rl::numeric T>
    constexpr auto format_as(const rect<T>& r)
    {
        return fmt::format("[{} | {}]", r.pt, r.size);
    }

#pragma pack()
}
