#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

#include "core/ds/dimensions.hpp"
#include "core/ds/point.hpp"
#include "core/ds/rect.hpp"
#include "core/ds/vector2d.hpp"
#include "core/numeric_types.hpp"
#include "core/utils/assert.hpp"
#include "core/utils/conversions.hpp"
#include "core/utils/io.hpp"
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

namespace SDL3
{
    inline static auto format_as(const SDL_RendererInfo& ri)
    {
        bool hw_accel = 0 != (ri.flags & SDL_RENDERER_ACCELERATED);
        bool software = 0 != (ri.flags & SDL_RENDERER_SOFTWARE);
        bool en_vsync = 0 != (ri.flags & SDL_RENDERER_PRESENTVSYNC);

        std::string buffer{};
        buffer.reserve(512);
        auto&& inserter = std::back_inserter(buffer);

        fmt::format_to(inserter, "Renderer Info:      \n");
        fmt::format_to(inserter, "  Name: {}          \n", ri.name);
        fmt::format_to(inserter, "  Max Texture Size: \n");
        fmt::format_to(inserter, "    Width:  {}      \n", ri.max_texture_width);
        fmt::format_to(inserter, "    Height: {}      \n", ri.max_texture_height);
        fmt::format_to(inserter, "  Context Flags:    \n");
        fmt::format_to(inserter, "    [{}] SDL_RENDERER_ACCELERATED \n", hw_accel ? "✓" : " ");
        fmt::format_to(inserter, "    [{}] SDL_RENDERER_SOFTWARE    \n", software ? "✓" : " ");
        fmt::format_to(inserter, "    [{}] SDL_RENDERER_PRESENTVSYNC\n", hw_accel ? "✓" : " ");
        fmt::format_to(inserter, "  Available Texture Formats: {}   \n", ri.num_texture_formats);
        for (rl::u32 i = 0; i < ri.num_texture_formats; ++i)
        {
            auto format = SDL3::SDL_GetPixelFormatName(ri.texture_formats[i]);
            fmt::format_to(inserter, "    {} \n", format);
        }
        return buffer;
    }
}

namespace rl::sdl
{
    class renderer
    {
        renderer()                      = delete;
        renderer(renderer&& other)      = delete;
        renderer(const renderer& other) = delete;

    public:
        renderer(const sdl::window& window, u32 flags)
            : m_sdl_renderer{ SDL3::SDL_CreateRenderer(window.get_handle(), nullptr, flags) }
        {
            runtime_assert(m_sdl_renderer != nullptr, "failed to create renderer");
            this->print_render_info();
        }

        renderer(const sdl::window& window, const std::string& name, u32 flags)
            : m_sdl_renderer{ SDL3::SDL_CreateRenderer(window.get_handle(), name.c_str(), flags) }
        {
            runtime_assert(m_sdl_renderer != nullptr, "failed to create renderer");
            this->print_render_info();
        }

        ~renderer()
        {
            if (m_sdl_renderer != nullptr)
                SDL3::SDL_DestroyRenderer(m_sdl_renderer);
        }

        void print_render_info()
        {
            if (m_sdl_renderer == nullptr)
            {
                auto style = fmt::emphasis::bold | fmt::fg(fmt::color::light_coral);
                fmt::print(style, "Renderer Invalid\n");
                return;
            }

            auto&& info{ this->get_info() };
            auto&& style = fmt::fg(fmt::color::light_steel_blue);
            fmt::print(style, "{}\n", info);
        }

        SDL3::SDL_Renderer* sdl_handle() const
        {
            runtime_assert(m_sdl_renderer != nullptr, "sdl renderer handle not initialized");
            return m_sdl_renderer;
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
            const i32 result = SDL3::SDL_RenderClear(m_sdl_renderer);
            runtime_assert(result == 0, "failed to clear renderer");
            return *this;
        }

        SDL3::SDL_RendererInfo get_info()
        {
            SDL3::SDL_RendererInfo info{};
            i32 result = SDL3::SDL_GetRendererInfo(m_sdl_renderer, &info);
            runtime_assert(result == 0, "failed to get renderer info");
            return info;
        }

        ds::dimensions<i32> get_output_size() const
        {
            ds::dimensions<i32> s{ 0, 0 };
            i32 result = SDL3::SDL_GetCurrentRenderOutputSize(m_sdl_renderer, &s.width, &s.height);
            runtime_assert(result == 0, "failed to get renderer output size");
            return s;
        }

        renderer& copy(texture& tex, const ds::rect<i32>& src_rect = ds::rect<i32>::null(),
                       const ds::rect<i32>& dst_rect = ds::rect<i32>::null())
        {
            const ds::rect<f32>& src_frect{ src_rect };
            const ds::rect<f32>& dst_frect{ dst_rect };
            i32 result = SDL3::SDL_RenderTexture(m_sdl_renderer, tex.sdl_handle(), src_frect,
                                                 dst_frect);
            runtime_assert(result == 0, "failed to copy texture");
            return *this;
        }

        renderer& copy(texture& tex, const ds::rect<i32>& src_rect, const ds::point<i32>& dst_pnt)
        {
            const auto&& tsize{ tex.size() };
            ds::rect<i32> dst_rect(dst_pnt.x,
                                   dst_pnt.y,
                                   src_rect.is_null() ? src_rect.width() : tsize.width,
                                   src_rect.is_null() ? src_rect.height() : tsize.height);
            return this->copy(tex, src_rect, dst_rect);
        }

        renderer& copy(texture& tex,
                       const ds::rect<i32>& src_rect,
                       const ds::rect<i32>& dst_rect,
                       f64 angle,
                       const ds::point<i32>& center_pt = ds::point<i32>{},
                       SDL3::SDL_RendererFlip flip     = SDL3::SDL_RendererFlip::SDL_FLIP_NONE)
        {
            const ds::rect<f32>& src_frect{ src_rect };
            const ds::rect<f32>& dst_frect{ dst_rect };
            const ds::point<f32>& center_fpt{ center_pt };
            i32 result = SDL3::SDL_RenderTextureRotated(m_sdl_renderer,
                                                        tex.sdl_handle(),
                                                        src_frect,
                                                        dst_frect,
                                                        angle,
                                                        center_fpt,
                                                        flip);

            runtime_assert(result == 0, "failed to copy rotated texture");
            return *this;
        }

        renderer& copy(texture& tex,
                       const ds::rect<i32>& src_rect,
                       const ds::point<i32>& dst_point,
                       f64 angle,
                       const ds::point<i32>& center_pt = ds::point<i32>{},
                       SDL3::SDL_RendererFlip flip     = SDL3::SDL_RendererFlip::SDL_FLIP_NONE)
        {
            auto&& tsize{ tex.size() };
            ds::rect<i32> dst_rect = {
                { dst_point.x, dst_point.y },
                {
                    (src_rect ? src_rect.width() : tsize.width),
                    (src_rect ? src_rect.height() : tsize.height),
                },
            };

            return this->copy(tex,
                              src_rect,
                              std::forward<ds::rect<i32> const&>(dst_rect),
                              angle,
                              std::forward<ds::point<i32> const&>(center_pt),
                              flip);
        }

        renderer& fill_copy(texture& tex,
                            const ds::rect<i32>& src_rect,
                            const ds::rect<i32>& dst_rect,
                            const ds::vector2<i32>& offset,
                            SDL3::SDL_RendererFlip flip)
        {
            // resolve rectangles
            ds::dimensions<i32> tsize{ tex.size() };
            ds::dimensions<i32> rsize{ this->get_output_size() };
            ds::rect<i32> src = { (
                src_rect.is_null() ? src_rect : ds::rect<i32>{ 0, 0, tsize.width, tsize.height }) };
            ds::rect<i32> dst = { (
                dst_rect.is_null() ? dst_rect : ds::rect<i32>{ 0, 0, rsize.width, rsize.height }) };

            // rectangle for single tile
            ds::rect<i32> start_tile{ offset.x, offset.y, src.width(), src.height() };

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

                        this->copy(tex,
                                   std::forward<ds::rect<i32>&>(tile_src),
                                   std::forward<ds::rect<i32>&>(tile_dst),
                                   0.0,
                                   std::forward<ds::point<i32>>(ds::point<i32>::null()),
                                   flip);
                    }
                    else
                    {
                        this->copy(tex,
                                   std::forward<ds::rect<i32>>(tile_src),
                                   std::forward<ds::rect<i32>>(tile_dst));
                    }
                }
            }

            return *this;
        }

        renderer& set_draw_color(sdl::color&& c)
        {
            i32 result = SDL3::SDL_SetRenderDrawColor(m_sdl_renderer, c.r, c.g, c.b, c.a);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& set_target()
        {
            i32 result = SDL3::SDL_SetRenderTarget(m_sdl_renderer, nullptr);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& set_target(sdl::texture& tex)
        {
            i32 result = SDL3::SDL_SetRenderTarget(m_sdl_renderer, tex.sdl_handle());
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& set_draw_blend_mode(SDL3::SDL_BlendMode blend_mode)
        {
            i32 result = SDL3::SDL_SetRenderDrawBlendMode(m_sdl_renderer, blend_mode);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_point(const ds::point<f32>& pt)
        {
            i32 result = SDL3::SDL_RenderPoint(m_sdl_renderer, pt.x, pt.y);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_points(const std::vector<ds::point<f32>>& points)
        {
            i32 count = cast::to<i32>(points.size());
            if (count > 0) [[likely]]
            {
                i32 result = SDL3::SDL_RenderPoints(m_sdl_renderer, points.front(), count);
                runtime_assert(result == 0, "failed to set draw color");
            }
            return *this;
        }

        renderer& draw_line(const ds::point<f32>& pt1, const ds::point<f32>& pt2)
        {
            i32 result = SDL3::SDL_RenderLine(m_sdl_renderer, pt1.x, pt1.y, pt2.x, pt2.y);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_lines(const std::vector<ds::point<f32>>& lines)
        {
            i32 count = cast::to<i32>(lines.size());
            if (count > 0) [[likely]]
            {
                i32 result = SDL3::SDL_RenderLines(m_sdl_renderer, lines.front(), count);
                runtime_assert(result == 0, "failed to set draw color");
            }
            return *this;
        }

        renderer& draw_rect(ds::rect<f32>& rect)
        {
            i32 result = SDL3::SDL_RenderRect(m_sdl_renderer, rect);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& draw_rects(const std::vector<ds::rect<f32>>& rects)
        {
            const i32 count = cast::to<i32>(rects.size());
            if (count > 0) [[likely]]
            {
                i32 result = SDL3::SDL_RenderRects(m_sdl_renderer, rects.front(), count);
                runtime_assert(result == 0, "failed to set draw color");
            }
            return *this;
        }

        renderer& fill_rect(const ds::rect<f32>& rect)
        {
            i32 result = SDL3::SDL_RenderFillRect(m_sdl_renderer, rect);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& fill_rects(const std::vector<ds::rect<f32>>& rects)
        {
            i32 count = cast::to<i32>(rects.size());
            if (count > 0) [[likely]]
            {
                i32 result = SDL3::SDL_RenderFillRects(m_sdl_renderer, rects.front(), count);
                runtime_assert(result == 0, "failed to set draw color");
            }
            return *this;
        }

        void read_pixels(const ds::rect<i32>& rect, u32 format, void* pixels, i32 pitch)
        {
            i32 result = SDL3::SDL_RenderReadPixels(m_sdl_renderer, rect, format, pixels, pitch);
            runtime_assert(result == 0, "failed to set draw color");
        }

        renderer& set_clip_rect(const ds::rect<i32>& rect)
        {
            i32 result = SDL3::SDL_SetRenderClipRect(m_sdl_renderer, rect);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& set_logical_size(i32 width, i32 height)
        {
            i32 result = SDL3::SDL_SetRenderLogicalPresentation(
                m_sdl_renderer,
                width,
                height,
                SDL3::SDL_LOGICAL_PRESENTATION_DISABLED,
                SDL3::SDL_SCALEMODE_NEAREST);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& set_scale(f32 scaleX, f32 scaleY)
        {
            i32 result = SDL3::SDL_SetRenderScale(m_sdl_renderer, scaleX, scaleY);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        renderer& set_viewport(const ds::rect<i32>& rect)
        {
            const SDL3::SDL_Rect& r = rect;
            i32 result              = SDL3::SDL_SetRenderViewport(m_sdl_renderer, &r);
            runtime_assert(result == 0, "failed to set draw color");
            return *this;
        }

        ds::rect<i32> get_clip_rect() const
        {
            SDL3::SDL_Rect rect{ 0, 0, 0, 0 };
            i32 result = SDL3::SDL_GetRenderClipRect(m_sdl_renderer, &rect);
            runtime_assert(result, "failed to set draw color");
            return rect;
        }

        ds::dimensions<i32> get_logical_size() const
        {
            ds::dimensions<i32> size{ 0, 0 };
            SDL3::SDL_GetRenderLogicalPresentation(m_sdl_renderer,
                                                   &size.width,
                                                   &size.height,
                                                   nullptr,
                                                   nullptr);
            return size;
        }

        ds::rect<i32> get_viewport()
        {
            ds::rect<i32> rect{ 0, 0, 0, 0 };
            SDL3::SDL_GetRenderViewport(m_sdl_renderer, reinterpret_cast<SDL3::SDL_Rect*>(&rect));
            return rect;
        }

        SDL3::SDL_BlendMode get_draw_blend_mode() const
        {
            SDL3::SDL_BlendMode mode{ SDL3::SDL_BLENDMODE_NONE };
            i32 result = SDL3::SDL_GetRenderDrawBlendMode(m_sdl_renderer, &mode);
            runtime_assert(result == 0, "failed to get renderer output size");
            return mode;
        }

        sdl::color get_draw_color() const
        {
            sdl::color c{};
            i32 result = SDL3::SDL_GetRenderDrawColor(m_sdl_renderer, &c.r, &c.g, &c.b, &c.a);
            runtime_assert(result == 0, "failed to get renderer output size");
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
