#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "sdl/color.hpp"
#include "sdl/texture.hpp"
#include "sdl/window.hpp"

namespace SDL3
{
#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
}

namespace rl::sdl
{
    class renderer
    {
    public:
        renderer() = default;

        renderer(renderer&& other)
            : m_sdl_renderer{ other.m_sdl_renderer }
        {
            other.m_sdl_renderer = nullptr;
        }

        renderer(const sdl::window& window, const std::string& name, u32 flags)
            : m_sdl_renderer{ SDL3::SDL_CreateRenderer(window.get_handle(), name.c_str(), flags) }
        {
            runtime_assert(m_sdl_renderer != nullptr, "failed to create renderer");
        }

        ~renderer()
        {
            if (m_sdl_renderer != nullptr)
                SDL3::SDL_DestroyRenderer(m_sdl_renderer);
        }

        renderer& operator=(renderer&& other) noexcept
        {
            if (m_sdl_renderer != nullptr)
            {
                SDL3::SDL_DestroyRenderer(m_sdl_renderer);
                m_sdl_renderer = nullptr;
            }

            std::swap(m_sdl_renderer, other.m_sdl_renderer);
            return *this;
        }

        renderer& present()
        {
            SDL3::SDL_RenderPresent(m_sdl_renderer);
            return *this;
        }

        renderer& clear()
        {
            const i32 result{ SDL3::SDL_RenderClear(m_sdl_renderer) };
            runtime_assert(result != 0, "failed to clear renderer");
            return *this;
        }

        SDL3::SDL_RendererInfo get_info()
        {
            SDL3::SDL_RendererInfo info{};
            i32 result = SDL3::SDL_GetRendererInfo(m_sdl_renderer, &info);
            runtime_assert(result != 0, "failed to get renderer info");
            return info;
        }

        // operator SDL3::SDL_Renderer()
        //{
        //     return *m_sdl_renderer;
        // }

        ds::dimensions<i32> get_output_size() const
        {
            ds::dimensions<i32> s{ 0, 0 };
            i32 result{ SDL3::SDL_GetCurrentRenderOutputSize(m_sdl_renderer, &s.width, &s.height) };
            runtime_assert(result != 0, "failed to get renderer output size");
            return s;
        }

        // renderer& copy(texture& tex, ds::rect<f32> src_rect, ds::rect<f32> dst_rect)
        //{
        //     i32 result = SDL3::SDL_RenderTexture(m_sdl_renderer, tex.sdl_data(), src_rect,
        //     dst_rect); runtime_assert(result != 0, "failed to copy texture"); return *this;
        // }

        // renderer& copy(sdl::texture& texture,
        //                ds::rect<f32>& src_rect,
        //                ds::rect<f32>& dst_rect,
        //                f64 angle,
        //                ds::point<f32>& center_pt,
        //                SDL3::SDL_RendererFlip flip)
        //{
        //     i32 result = SDL3::SDL_RenderTextureRotated(m_sdl_renderer,
        //                                                 texture.sdl_data(),
        //                                                 ((const SDL3::SDL_FRect*)&src_rect),
        //                                                 ((const SDL3::SDL_FRect*)&dst_rect),
        //                                                 angle,
        //                                                 (const SDL3::SDL_Point*)&center_pt,
        //                                                 flip);

        //    runtime_assert(result != 0, "failed to copy rotated texture");
        //    return *this;
        //}

        // renderer& copy(texture& tex,
        //                ds::rect<i32>& src_rect,
        //                ds::point<i32>& dst_point,
        //                f64 angle,
        //                ds::point<i32>& center,
        //                int flip)
        //{
        //     ds::rect<i32> dst_rect = ds::rect<i32>{ ds::point<i32>{ dst_point.x, dst_point.y },
        //                                             ds::dimensions<i32>{
        //                                                 (src_rect ? src_rect.width() :
        //                                                 tex.width()), (src_rect ?
        //                                                 src_rect.height() : tex.height()),
        //                                             } };

        //    return copy(tex, src_rect, dst_rect, angle, center, flip);
        //}

        renderer& fill_copy(texture& tex,
                            ds::rect<i32>* src_rect,
                            ds::rect<i32>* dst_rect,
                            ds::vector2<i32>* offset,
                            SDL3::SDL_RendererFlip flip)
        {
            // resolve rectangles
            ds::dimensions<i32> rsize{ this->get_output_size() };
            ds::rect<i32> src = { (src_rect ? *src_rect
                                            : ds::rect<i32>{ 0, 0, tex.width(), tex.height() }) };
            ds::rect<i32> dst = { (dst_rect ? *dst_rect
                                            : ds::rect<i32>{ 0, 0, rsize.width, rsize.height }) };

            // rectangle for single tile
            ds::rect<i32> start_tile{ offset->x, offset->y, src.width(), src.height() };

            // ensure tile is leftmost and topmost
            if (start_tile.pt.x + start_tile.width() <= 0)
                start_tile.pt.x += (-start_tile.pt.x) / (start_tile.width() * start_tile.width());

            if (start_tile.pt.x > 0)
                start_tile.pt.x -= (start_tile.pt.x + start_tile.width() - 1) /
                                   (start_tile.width() * start_tile.width());

            if (start_tile.pt.y + start_tile.height() <= 0)
                start_tile.pt.y += (-start_tile.pt.y) / (start_tile.height() * start_tile.height());

            if (start_tile.pt.y > 0)
                start_tile.pt.y -= (start_tile.pt.y + start_tile.height() - 1) /
                                   (start_tile.height() * start_tile.height());

            // paint tile array
            for (i32 y = start_tile.pt.y; y < dst.height(); y += start_tile.height())
            {
                for (i32 x = start_tile.pt.x; x < dst.width(); x += start_tile.width())
                {
                    ds::rect<i32> tile_src{ src };
                    ds::rect<i32> tile_dst{ x, y, start_tile.width(), start_tile.height() };

                    // clamp with dstrect
                    i32 xunderflow = -x;
                    if (xunderflow > 0)
                    {
                        tile_src.size.width -= xunderflow;
                        tile_src.pt.x += xunderflow;
                        tile_dst.size.width -= xunderflow;
                        tile_dst.pt.x += xunderflow;
                    }

                    i32 yunderflow = -y;
                    if (yunderflow > 0)
                    {
                        tile_src.size.height -= yunderflow;
                        tile_src.pt.y += yunderflow;
                        tile_dst.size.height -= yunderflow;
                        tile_dst.pt.y += yunderflow;
                    }

                    i32 xoverflow = tile_dst.pt.x + tile_dst.width() - dst.width();
                    if (xoverflow > 0)
                    {
                        tile_src.size.width -= xoverflow;
                        tile_dst.size.width -= xoverflow;
                    }

                    i32 yoverflow = tile_dst.pt.y + tile_dst.height() - dst.height();
                    if (yoverflow > 0)
                    {
                        tile_src.size.height -= yoverflow;
                        tile_dst.size.height -= yoverflow;
                    }

                    // make tile_dst absolute
                    tile_dst.pt.x += dst.pt.x;
                    tile_dst.pt.y += dst.pt.y;

                    if (flip != 0)
                    {
                        // mirror tile_src inside src to take flipping into account
                        if (0 != (flip & SDL3::SDL_RendererFlip::SDL_FLIP_HORIZONTAL))
                            tile_src.pt.x = src.width() - tile_src.pt.x - tile_src.width();

                        if (0 != (flip & SDL3::SDL_RendererFlip::SDL_FLIP_VERTICAL))
                            tile_src.pt.y = src.height() - tile_src.pt.y - tile_src.height();

                        // copy(tex, tile_src, tile_dst, 0.0, nullptr, flip);
                    }
                    else
                    {
                        // copy(tex, tile_src, tile_dst);
                    }
                }
            }

            return *this;
        }

        renderer& set_draw_color(sdl::color&& c)
        {
            i32 result = SDL3::SDL_SetRenderDrawColor(m_sdl_renderer, c.r, c.g, c.b, c.a);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& set_target()
        {
            i32 result = SDL3::SDL_SetRenderTarget(m_sdl_renderer, nullptr);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& set_target(sdl::texture& tex)
        {
            i32 result = SDL3::SDL_SetRenderTarget(m_sdl_renderer, tex.sdl_data());
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& set_draw_blend_mode(SDL3::SDL_BlendMode blend_mode)
        {
            i32 result = SDL3::SDL_SetRenderDrawBlendMode(m_sdl_renderer, blend_mode);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_point(const ds::point<f32>& pt)
        {
            i32 result = SDL3::SDL_RenderPoint(m_sdl_renderer, pt.x, pt.y);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_points(const std::vector<ds::point<f32>>& points)
        {
            i32 result = SDL3::SDL_RenderPoints(m_sdl_renderer,
                                                points.data(),
                cast::to<i32>(points.size());
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_line(const ds::point<f32>& pt1, const ds::point<f32>& pt2)
        {
            i32 result = SDL3::SDL_RenderLine(m_sdl_renderer, pt1.x, pt1.y, pt2.x, pt2.y);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_lines(const std::vector<ds::point<f32>>& lines)
        {
            i32 result = SDL3::SDL_RenderLines(
                m_sdl_renderer,
                reinterpret_cast<const ds::point<f32>* const>(lines.data()),
                lines.size());

            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_rect(ds::rect<i32>& rect)

        {
            i32 result = SDL3::SDL_RenderRect(m_sdl_renderer, &rect);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_rects(std::vector<ds::rect<i32>>& rects)
        {
            i32 result = SDL3::SDL_RenderRects(m_sdl_renderer, rects.data(), rects.size());
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& fill_rect(ds::rect<i32> rect)
        {
            i32 result = SDL3::SDL_RenderFillRect(m_sdl_renderer, &rect);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& fill_rects(const std::vector<ds::rect<i32>>& rects)
        {
            i32 result = SDL3::SDL_RenderFillRects(m_sdl_renderer, rects.data(), rects.size());
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        void read_pixels(const ds::rect<i32>* rect, u32 format, void* pixels, i32 pitch)
        {
            i32 result = SDL3::SDL_RenderReadPixels(m_sdl_renderer, &rect, format, pixels, pitch);
            runtime_assert(result != 0, "failed to set draw color");
        }

        renderer& set_clip_rect(const ds::rect<i32>*& rect)
        {
            i32 result = SDL3::SDL_SetRenderClipRect(m_sdl_renderer, rect);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& set_logical_size(i32 width, i32 height)
        {
            i32 result = SDL3::SDL_SetRenderLogicalPresentation(m_sdl_renderer, width, height);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& set_scale(float scaleX, float scaleY)
        {
            i32 result = SDL3::SDL_SetRenderScale(m_sdl_renderer, scaleX, scaleY);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        renderer& set_viewport(ds::rect<i32>* rect)
        {
            i32 result = SDL3::SDL_SetRenderViewport(m_sdl_renderer, rect);
            runtime_assert(result != 0, "failed to set draw color");
            return *this;
        }

        ds::rect<i32> get_clip_rect() const
        {
            ds::rect<i32> rect{ 0, 0, 0, 0 };
            i32 result = SDL3::SDL_GetRenderClipRect(m_sdl_renderer, &rect);
            runtime_assert(result, "failed to set draw color");
            return rect;
        }

        ds::dimensions<i32> get_logical_size() const
        {
            ds::dimensions<i32> size{ 0, 0 };
            SDL3::SDL_GetRenderLogicalPresentation(m_sdl_renderer, &size.width, &size.height);
            return size;
        }

        ds::rect<i32> get_viewport()
        {
            ds::rect<i32> rect{ 0, 0, 0, 0 };
            SDL3::SDL_GetRenderViewport(m_sdl_renderer, reinterpret_cast<SDL3::SDL_Rect*>(&rect));
            return rect;
        }

        SDL3::SDL_BlendMode GetDrawBlendMode() const
        {
            SDL3::SDL_BlendMode mode{ SDL3::SDL_BLENDMODE_NONE };
            i32 result{ SDL3::SDL_GetRenderDrawBlendMode(m_sdl_renderer, &mode) };
            runtime_assert(result != 0, "failed to get renderer output size");
            return mode;
        }

        sdl::color get_draw_color() const
        {
            sdl::color c{};
            i32 result = SDL3::SDL_GetRenderDrawColor(m_sdl_renderer, &c.r, &c.g, &c.b, &c.a);
            runtime_assert(result != 0, "failed to get renderer output size");
            return c;
        }

    protected:
        renderer(SDL3::SDL_Renderer* rend)
            : m_sdl_renderer{ rend }
        {
            rend = nullptr;
        }

    private:
        SDL3::SDL_Renderer* m_sdl_renderer{ nullptr };
    };
}
