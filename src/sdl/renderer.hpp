#pragma once

#include <bitset>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "core/numeric.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "sdl/color.hpp"
#include "sdl/defs.hpp"
#include "sdl/texture.hpp"
#include "sdl/window.hpp"
#include "utils/assert.hpp"
#include "utils/conversions.hpp"
#include "utils/io.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

namespace rl::sdl {
    class Texture;
    class Window;

    class Renderer
    {
    public:
        struct Properties : public std::bitset<32>
        {
            using sdl_type = SDL3::SDL_RendererFlags;

            enum Flag : std::underlying_type_t<sdl_type> {
                None = 0,
                Software = SDL3::SDL_RENDERER_SOFTWARE,
                HWAccelerated = SDL3::SDL_RENDERER_ACCELERATED,
                VSync = SDL3::SDL_RENDERER_PRESENTVSYNC,
            };

            constexpr inline operator sdl_type()
            {
                return static_cast<sdl_type>(this->to_ulong());
            }
        };

        struct driver
        {
            constexpr static inline std::string_view DirectX{ "direct3d" };
            constexpr static inline std::string_view DirectX11{ "direct3d11" };
            constexpr static inline std::string_view DirectX12{ "direct3d12" };
            constexpr static inline std::string_view OpenGL{ "opengl" };
            constexpr static inline std::string_view OpenGLES2{ "opengles2" };
            constexpr static inline std::string_view OpenGLES{ "opengles" };
            constexpr static inline std::string_view Metal{ "metal" };
            constexpr static inline std::string_view Software{ "software" };
        };

        struct blend_mode
        {
            using type = SDL3::SDL_BlendMode;
            constexpr static inline auto None = SDL3::SDL_BLENDMODE_NONE;
            constexpr static inline auto Blend = SDL3::SDL_BLENDMODE_BLEND;
            constexpr static inline auto Add = SDL3::SDL_BLENDMODE_ADD;
            constexpr static inline auto Mod = SDL3::SDL_BLENDMODE_MOD;
            constexpr static inline auto Mul = SDL3::SDL_BLENDMODE_MUL;
            constexpr static inline auto Invalid = SDL3::SDL_BLENDMODE_INVALID;
        };

        constexpr static inline auto DEFAULT_GRAPHICS_DRIVER = driver::OpenGL;
        constexpr static inline Properties DEFAULT_PROPERTY_FLAGS = {
            Properties::HWAccelerated | Properties::VSync,
        };

        Renderer() = delete;
        Renderer(Renderer& other) = delete;
        Renderer(const Renderer& other) = delete;

        explicit Renderer(const sdl::Window& window, std::string_view driver,
                          Renderer::Properties flags)
            : m_properties{ flags }
            , m_sdl_renderer{ SDL3::SDL_CreateRenderer(window.sdl_handle(), driver.data(),
                                                       m_properties) }
        {
            sdl_assert(m_sdl_renderer != nullptr, "failed to create renderer");
            this->print_render_info();
        }

        ~Renderer()
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

        bool is_valid() const
        {
            return this->sdl_handle() != nullptr;
        }

        SDL3::SDL_Renderer* sdl_handle() const
        {
            sdl_assert(m_sdl_renderer != nullptr, "sdl renderer handle not initialized");
            return m_sdl_renderer;
        }

        static std::string current_video_driver()
        {
            return std::string{ SDL3::SDL_GetCurrentVideoDriver() };
        }

        static std::vector<std::string> get_render_drivers()
        {
            std::vector<std::string> render_drivers{};
            i32 count = SDL3::SDL_GetNumRenderDrivers();
            sdl_assert(count >= 0, "failed retreiving render drivers");

            render_drivers.reserve(count);
            for (i32 i = 0; i < count; ++i)
                render_drivers.emplace_back(SDL3::SDL_GetRenderDriver(i));

            return render_drivers;
        }

        Renderer& operator=(Renderer&& other) noexcept
        {
            if (m_sdl_renderer != nullptr)
            {
                SDL3::SDL_DestroyRenderer(m_sdl_renderer);
                m_sdl_renderer = nullptr;
            }

            std::swap(m_sdl_renderer, other.m_sdl_renderer);
            return *this;
        }

        bool present()
        {
            i32 result = SDL3::SDL_RenderPresent(m_sdl_renderer);
            sdl_assert(result == 0, "failed to present renderer");
            return result == 0;
        }

        bool clear(const sdl::Color<u8>& c = { 0, 0, 0 })
        {
            bool ret = this->set_draw_color(c);
            ret &= 0 == SDL3::SDL_RenderClear(m_sdl_renderer);
            sdl_assert(ret, "failed to clear renderer");
            return ret;
        }

        SDL3::SDL_RendererInfo get_info()
        {
            SDL3::SDL_RendererInfo info{};
            i32 result = SDL3::SDL_GetRendererInfo(m_sdl_renderer, &info);
            sdl_assert(result == 0, "failed to get renderer info");
            return info;
        }

        /**
         * @brief returns the size of the current rendering context
         * */
        ds::dims<i32> get_output_size() const
        {
            ds::dims<i32> s{ 0, 0 };
            i32 result = SDL3::SDL_GetCurrentRenderOutputSize(m_sdl_renderer, &s.width, &s.height);
            sdl_assert(result == 0, "failed to get renderer output size");
            return s;
        }

        bool copy(sdl::Texture& tex, const ds::rect<i32>& src_rect = ds::rect<i32>::null(),
                  const ds::rect<i32>& dst_rect = ds::rect<i32>::null())
        {
            const ds::rect<f32>& src_frect{ src_rect };
            const ds::rect<f32>& dst_frect{ dst_rect };
            i32 result = SDL3::SDL_RenderTexture(m_sdl_renderer, tex.sdl_handle(), src_frect,
                                                 dst_frect);
            sdl_assert(result == 0, "failed to copy texture");
            return result == 0;
        }

        bool copy(Texture& tex, const ds::rect<i32>& src_rect, const ds::point<i32>& dst_pnt)
        {
            const auto&& tsize{ tex.size() };
            ds::rect<i32> dst_rect(dst_pnt.x, dst_pnt.y,
                                   src_rect.is_null() ? src_rect.size.width : tsize.width,
                                   src_rect.is_null() ? src_rect.size.height : tsize.height);
            return this->copy(tex, src_rect, dst_rect);
        }

        bool copy(Texture& tex, const ds::rect<i32>& src_rect, const ds::rect<i32>& dst_rect,
                  f64 angle, const ds::point<i32>& center_pt = ds::point<i32>{},
                  SDL3::SDL_RendererFlip flip = SDL3::SDL_RendererFlip::SDL_FLIP_NONE)
        {
            const ds::rect<f32>& src_frect{ src_rect };
            const ds::rect<f32>& dst_frect{ dst_rect };
            const ds::point<f32>& center_fpt{ center_pt };
            i32 result = SDL3::SDL_RenderTextureRotated(m_sdl_renderer, tex.sdl_handle(), src_frect,
                                                        dst_frect, angle, center_fpt, flip);

            sdl_assert(result == 0, "failed to copy rotated texture");
            return result == 0;
        }

        bool copy(Texture& tex, const ds::rect<i32>& src_rect, const ds::point<i32>& dst_point,
                  f64 angle, const ds::point<i32>& center_pt = ds::point<i32>::null(),
                  SDL3::SDL_RendererFlip flip = SDL3::SDL_RendererFlip::SDL_FLIP_NONE)
        {
            auto&& tsize{ tex.size() };
            ds::rect<i32> dst_rect = {
                ds::point<i32>{
                    dst_point.x,
                    dst_point.y,
                },
                ds::dims<i32>{
                    src_rect ? src_rect.size.width : tsize.width,
                    src_rect ? src_rect.size.height : tsize.height,
                },
            };

            return this->copy(tex, src_rect, std::forward<const ds::rect<i32>&>(dst_rect), angle,
                              std::forward<const ds::point<i32>&>(center_pt), flip);
        }

        bool fill_copy(Texture& tex, const ds::rect<i32>& src_rect, const ds::rect<i32>& dst_rect,
                       const ds::vector2<i32>& offset, SDL3::SDL_RendererFlip flip)
        {
            bool ret = true;

            // resolve rectangles
            ds::dims<i32> tsize{ tex.size() };
            ds::dims<i32> rsize{ this->get_output_size() };
            ds::rect<i32> src = { (
                src_rect.is_null() ? src_rect : ds::rect<i32>{ 0, 0, tsize.width, tsize.height }) };
            ds::rect<i32> dst = { (
                dst_rect.is_null() ? dst_rect : ds::rect<i32>{ 0, 0, rsize.width, rsize.height }) };

            // rectangle for single tile
            ds::rect<i32> start_tile{ offset.x, offset.y, src.size.width, src.size.height };

            // ensure tile is leftmost and topmost
            if (start_tile.pt.x + start_tile.size.width <= 0)
                start_tile.pt.x += (-start_tile.pt.x) /
                                   (start_tile.size.width * start_tile.size.width);

            if (start_tile.pt.x > 0)
                start_tile.pt.x -= (start_tile.pt.x + start_tile.size.width - 1) /
                                   (start_tile.size.width * start_tile.size.width);

            if (start_tile.pt.y + start_tile.size.height <= 0)
                start_tile.pt.y += (-start_tile.pt.y) /
                                   (start_tile.size.height * start_tile.size.height);

            if (start_tile.pt.y > 0)
                start_tile.pt.y -= (start_tile.pt.y + start_tile.size.height - 1) /
                                   (start_tile.size.height * start_tile.size.height);

            // paint tile array
            for (i32 y = start_tile.pt.y; y < dst.size.height; y += start_tile.size.height)
            {
                for (i32 x = start_tile.pt.x; x < dst.size.width; x += start_tile.size.width)
                {
                    ds::rect<i32> tile_src{ src };
                    ds::rect<i32> tile_dst{ x, y, start_tile.size.width, start_tile.size.height };

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

                    i32 xoverflow = tile_dst.pt.x + tile_dst.size.width - dst.size.width;
                    if (xoverflow > 0)
                    {
                        tile_src.size.width -= xoverflow;
                        tile_dst.size.width -= xoverflow;
                    }

                    i32 yoverflow = tile_dst.pt.y + tile_dst.size.height - dst.size.height;
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
                            tile_src.pt.x = src.size.width - tile_src.pt.x - tile_src.size.width;

                        if (0 != (flip & SDL3::SDL_RendererFlip::SDL_FLIP_VERTICAL))
                            tile_src.pt.y = src.size.height - tile_src.pt.y - tile_src.size.height;

                        ret |= this->copy(tex, std::forward<ds::rect<i32>&>(tile_src),
                                          std::forward<ds::rect<i32>&>(tile_dst), 0.0,
                                          std::forward<ds::point<i32>>(ds::point<i32>::null()),
                                          flip);
                    }
                    else
                    {
                        ret |= this->copy(tex, std::forward<ds::rect<i32>>(tile_src),
                                          std::forward<ds::rect<i32>>(tile_dst));
                    }
                }
            }

            return ret;
        }

        bool set_draw_color(const sdl::Color<u8>& c)
        {
            i32 result = SDL3::SDL_SetRenderDrawColor(m_sdl_renderer, c.r, c.g, c.b, c.a);
            sdl_assert(result == 0, "failed to set draw color");
            return result == 0;
        }

        bool draw_texture(sdl::Texture& texture,
                          const ds::rect<f32>& src_rect = ds::rect<f32>::null(),
                          const ds::rect<f32>& dst_rect = ds::rect<f32>::null())
        {
            i32 result = SDL3::SDL_RenderTexture(m_sdl_renderer, texture.sdl_handle(), src_rect,
                                                 dst_rect);
            sdl_assert(result == 0, "failed to draw texture");
            return result == 0;
        }

        bool set_target()
        {
            i32 result = SDL3::SDL_SetRenderTarget(m_sdl_renderer, nullptr);
            sdl_assert(result == 0, "failed to reset render target");
            return result == 0;
        }

        bool set_target(sdl::Texture& tex)
        {
            i32 result = SDL3::SDL_SetRenderTarget(m_sdl_renderer, tex.sdl_handle());
            sdl_assert(result == 0, "failed to set render target");
            return result == 0;
        }

        bool set_draw_blend_mode(const SDL3::SDL_BlendMode blend_mode)
        {
            i32 result = SDL3::SDL_SetRenderDrawBlendMode(m_sdl_renderer, blend_mode);
            sdl_assert(result == 0, "failed to set draw blend mode");
            return result == 0;
        }

        bool draw_point(const ds::point<f32>& pt)
        {
            i32 result = SDL3::SDL_RenderPoint(m_sdl_renderer, pt.x, pt.y);
            sdl_assert(result == 0, "failed to draw point");
            return result == 0;
        }

        bool draw_points(const std::vector<ds::point<f32>>& points)
        {
            i32 result = 0;

            const i32 count{ cast::to<i32>(points.size()) };
            if (count > 0) [[likely]]
            {
                result = SDL3::SDL_RenderPoints(m_sdl_renderer, points.front(), count);
                sdl_assert(result == 0, "failed to draw points");
            }

            return result == 0;
        }

        bool draw_line(const ds::point<f32>& pt1, const ds::point<f32>& pt2)
        {
            i32 result = SDL3::SDL_RenderLine(m_sdl_renderer, pt1.x, pt1.y, pt2.x, pt2.y);
            sdl_assert(result == 0, "failed to draw line");
            return result == 0;
        }

        bool draw_lines(const std::vector<ds::point<f32>>& lines)
        {
            i32 result = 0;

            const i32 count{ cast::to<i32>(lines.size()) };
            if (count > 0) [[likely]]
            {
                result = SDL3::SDL_RenderLines(m_sdl_renderer, lines.front(), count);
                sdl_assert(result == 0, "failed to draw lines");
            }

            return result == 0;
        }

        bool draw_rect(ds::rect<f32>&& rect, const sdl::Color<u8>& c = {})
        {
            i32 result = 0;
            if (!c.is_empty())
                this->set_draw_color(c);

            result |= SDL3::SDL_RenderRect(m_sdl_renderer, rect);
            sdl_assert(result == 0, "failed to draw rect");
            return result == 0;
        }

        bool draw_rects(const std::vector<ds::rect<f32>>& rects)
        {
            i32 result = 0;

            const i32 count{ cast::to<i32>(rects.size()) };
            if (count > 0) [[likely]]
            {
                result = SDL3::SDL_RenderRects(m_sdl_renderer, rects.front(), count);
                sdl_assert(result == 0, "failed to draw rects");
            }
            return result == 0;
        }

        bool fill_rect(const ds::rect<f32>& rect = ds::rect<i32>::null(),
                       const sdl::Color<u8>& c = {})
        {
            i32 result = 0;
            if (!c.is_empty())
                this->set_draw_color(c);

            result |= SDL3::SDL_RenderFillRect(m_sdl_renderer, rect);
            sdl_assert(result == 0, "failed to fill rect");
            return result == 0;
        }

        bool fill_rects(const std::vector<ds::rect<f32>>& rects, const sdl::Color<u8>& c = {})
        {
            i32 result = 0;

            i32 count = cast::to<i32>(rects.size());
            if (count > 0) [[likely]]
            {
                if (!c.is_empty())
                    this->set_draw_color(c);

                result = SDL3::SDL_RenderFillRects(m_sdl_renderer, rects.front(), count);
                sdl_assert(result == 0, "failed to fill rects");
            }

            return result == 0;
        }

        bool fill_rects(const std::vector<std::pair<ds::rect<f32>, sdl::Color<u8>>>& rects)
        {
            bool ret = true;

            for (auto&& [r, c] : rects)
                ret &= this->fill_rect(r, c);

            return ret;
        }

        bool read_pixels(const ds::rect<i32>& rect, SDL3::SDL_PixelFormatEnum format, void* pixels,
                         i32 pitch)
        {
            i32 result = SDL3::SDL_RenderReadPixels(m_sdl_renderer, rect, format, pixels, pitch);
            sdl_assert(result == 0, "failed to read pixels");
            return result == 0;
        }

        bool set_clip_rect(const ds::rect<i32>& rect)
        {
            i32 result = SDL3::SDL_SetRenderClipRect(m_sdl_renderer, rect);
            sdl_assert(result == 0, "failed to set clip rect");
            return result == 0;
        }

        bool set_logical_size(i32 width, i32 height,
                              SDL3::SDL_RendererLogicalPresentation presentation =
                                  SDL3::SDL_LOGICAL_PRESENTATION_DISABLED,
                              SDL3::SDL_ScaleMode scale_mode = SDL3::SDL_SCALEMODE_NEAREST)
        {
            i32 result = SDL3::SDL_SetRenderLogicalPresentation(m_sdl_renderer, width, height,
                                                                presentation, scale_mode);
            sdl_assert(result == 0, "failed to set logical size");
            return result == 0;
        }

        bool set_logical_size(const ds::dims<i32>& size,
                              SDL3::SDL_RendererLogicalPresentation presentation =
                                  SDL3::SDL_LOGICAL_PRESENTATION_DISABLED,
                              SDL3::SDL_ScaleMode scale_mode = SDL3::SDL_SCALEMODE_NEAREST)
        {
            return this->set_logical_size(size.width, size.height, presentation, scale_mode);
        }

        bool set_scale(f32 scale_x, f32 scale_y)
        {
            i32 result = SDL3::SDL_SetRenderScale(m_sdl_renderer, scale_x, scale_y);
            sdl_assert(result == 0, "failed to set scale");
            return result == 0;
        }

        bool set_scale(const ds::vector2<f32>& scale)
        {
            return this->set_scale(scale.x, scale.y);
        }

        bool set_viewport(const ds::rect<i32>& rect = ds::rect<i32>::null())
        {
            i32 result = SDL3::SDL_SetRenderViewport(m_sdl_renderer, rect);
            sdl_assert(result == 0, "failed to set viewport");
            return result == 0;
        }

        ds::rect<i32> get_clip_rect() const
        {
            ds::rect<i32> rect{ 0, 0, 0, 0 };
            i32 result = SDL3::SDL_GetRenderClipRect(m_sdl_renderer, rect);
            sdl_assert(result, "failed to get clip rect");
            return rect;
        }

        /**
         * @brief Returns rendering output size
         * */
        ds::dims<i32> get_logical_size() const
        {
            ds::dims<i32> size{ 0, 0 };
            i32 result = SDL3::SDL_GetRenderLogicalPresentation(m_sdl_renderer, &size.width,
                                                                &size.height, nullptr, nullptr);
            sdl_assert(result == 0, "failed to get logical size");
            return size;
        }

        ds::rect<i32> get_viewport()
        {
            ds::rect<i32> rect{ 0, 0, 0, 0 };
            i32 result = SDL3::SDL_GetRenderViewport(m_sdl_renderer, rect);
            sdl_assert(result == 0, "failed to get viewport");
            return rect;
        }

        SDL3::SDL_BlendMode get_draw_blend_mode() const
        {
            SDL3::SDL_BlendMode mode{ SDL3::SDL_BLENDMODE_NONE };
            i32 result = SDL3::SDL_GetRenderDrawBlendMode(m_sdl_renderer, &mode);
            sdl_assert(result == 0, "failed to get draw blend mode");
            return mode;
        }

        sdl::Color<u8> get_draw_color() const
        {
            sdl::Color<u8> c{};
            i32 result = SDL3::SDL_GetRenderDrawColor(m_sdl_renderer, &c.r, &c.g, &c.b, &c.a);
            sdl_assert(result == 0, "failed to get draw color");
            return c;
        }

    private:
        Properties m_properties{ Properties::None };
        SDL3::SDL_Renderer* m_sdl_renderer{ nullptr };
    };
}

SDL_C_LIB_BEGIN

static inline auto format_as(const SDL_RendererInfo& ri)
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

SDL_C_LIB_END
