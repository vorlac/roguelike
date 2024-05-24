#pragma once

#include "ds/rect.hpp"

namespace rl::test3 {
    ds::rect<f32> zoom_to_point(ds::rect<f32> orig_rect, ds::point<f32> zoom_pos, f32 zoom_factor)
    {
        const ds::rect new_rect{
            // first scale the rectangle by the zoom factor,
            // then center the scaled rect at the zoom point
            orig_rect.scaled(1.0f / zoom_factor).center(zoom_pos)
        };

        fmt::println("zoom_factor = {}", zoom_factor);
        fmt::println("zoom_pt     = {}", zoom_pos);
        fmt::println("orig_rect   = {}", orig_rect);
        fmt::println("new_rect    = {}", new_rect);
        fmt::println("");

        // compute the difference between
        // the orig and new rectangle...
        const ds::rect<f32> delta{
            new_rect.pt - orig_rect.pt,
            new_rect.size - orig_rect.size,
        };

        // interpolate the rect's size shift if you want a
        // smoother transition from the start to end rects
        ds::rect<f32> inc_rect{ ds::rect<f32>::zero() };
        constexpr static i32 incremental_zoom_count{ 10 };
        for (i32 i = 0; i < incremental_zoom_count - 1; ++i) {
            inc_rect = {
                orig_rect.pt + (delta.pt * (i + 1.0f)) / incremental_zoom_count,
                orig_rect.size + (delta.size * (i + 1.0f)) / incremental_zoom_count,
            };
            fmt::println("incremental move {} = {}", i, inc_rect);

            // update map incrementally
            // for smoother transition
            /* set_map_rect(inc_rect); */
        }

        fmt::println("final rect = {}", new_rect);
        fmt::println("");

        // update map to frame new rect
        /* set_map_rect(new_rect); */

        return new_rect;
    }

    int zoomtest()
    {
        // zoom in 50%
        // (area that's visible will be 2/3 what it was)
        constexpr f32 zoom_factor1{ 1.5f };
        constexpr ds::point<f32> zoom_pos1{ 500.0f, 500.0f };
        constexpr ds::rect<f32> orig_rect1{
            ds::point<f32>{ 0.0f, 0.0f },
            ds::dims<f32>{ 1000.0f, 1000.0f }
        };

        const ds::rect<f32> orig_rect2{
            zoom_to_point(orig_rect1, zoom_pos1,
                          zoom_factor1)
        };

        // zoom out 2x
        // (area that's visible will be 2x what it was)
        constexpr f32 zoom_factor2{ 0.5f };
        constexpr ds::point<f32> zoom_pos2{ 250.0f, 250.0f };

        const ds::rect<f32> final_rect{
            zoom_to_point(orig_rect2, zoom_pos2,
                          zoom_factor2)
        };
    }
}
