#pragma once

#include "ds/rect.hpp"
#include "gfx/text.hpp"
#include "gfx/vg/nanovg.hpp"
#include "ui/theme.hpp"
#include "utils/numeric.hpp"

namespace rl {
    namespace nvg {
        struct PaintStyle;
    }

    struct TextProperties
    {
        std::string_view font{};
        Align align{ Align::None };
        ds::color<f32> color{ Colors::Transparent };
        f32 font_size{ -1.0f };
    };

    class NVGRenderer
    {
    public:
        NVGRenderer();

        [[nodiscard]] nvg::Context* context() const;

        [[nodiscard]] nvg::PaintStyle create_rect_gradient_paint_style(
            const ds::rect<f32>& rect, f32 corner_radius, f32 outer_blur,
            const ds::color<f32>& inner_color, const ds::color<f32>& outer_gradient_color) const;

        [[nodiscard]] nvg::PaintStyle create_linear_gradient_paint_style(
            ds::line<f32> line, const ds::color<f32>& inner_color,
            const ds::color<f32>& outer_gradient_color) const;

        [[nodiscard]] text::font::handle load_font(
            const std::string_view& font_name,
            const std::basic_string_view<u8>& font_ttf) const;

        [[nodiscard]] ds::dims<f32> get_text_size(
            const std::string& text) const;

        [[nodiscard]] ds::dims<f32> get_text_size(
            const std::string& text, const std::string_view& font_name,
            f32 font_size, Align alignment = Align::HCenter | Align::VMiddle) const;

        [[nodiscard]] ds::rect<f32> get_text_box_rect(
            const std::string& text, ds::point<f32> pos, std::string_view font_name,
            f32 font_size, f32 fold_width, Align alignment = Align::HLeft | Align::VTop) const;

        void set_fill_paint_style(const nvg::PaintStyle& paint_style) const;
        void fill_current_path(const nvg::PaintStyle& paint_style) const;

        void set_text_properties(const TextProperties& props) const;
        void set_text_properties(std::string_view font_name,
                                 f32 font_size = -1.0f,
                                 Align alignment = Align::None,
                                 const ds::color<f32>& text_color = Colors::Transparent) const;

        void draw_text(std::string text, ds::point<f32> pos,
                       const TextProperties& props = {}) const;
        void draw_rounded_rect(const ds::rect<f32>& rect, f32 corner_radius) const;
        void draw_rect_outline(const ds::rect<f32>& rect, f32 stroke_width,
                               const ds::color<f32>& color, Outline type) const;

        void load_fonts(const std::vector<text::font::Data>& fonts);
        void begin_frame(const ds::dims<f32>& render_size, f32 pixel_ratio = 1.0f) const;
        void end_frame() const;

        void save_state() const;
        void restore_state() const;
        void reset_scissor() const;
        void flush(const ds::dims<f32>& viewport, f32 pixel_ratio = 1.0f) const;

        void begin_path() const;
        void end_path() const;

    public:
        template <std::invocable TCallable>
        void scoped_draw(TCallable&& callable)
        {
            this->save_state();
            std::invoke(std::forward<TCallable>(callable));
            this->restore_state();
        }

        template <std::invocable TCallable>
        void draw_frame(TCallable&& callable, const ds::dims<f32>& render_size, f32 pixel_ratio)
        {
            this->begin_frame(render_size, pixel_ratio);
            std::invoke(std::forward<TCallable>(callable));
            this->end_frame();
        }

        template <std::invocable TCallable>
        void draw_path(const bool close_when_done, TCallable&& callable)
        {
            this->begin_path();
            std::invoke(std::forward<TCallable>(callable));
            if (close_when_done)
                this->end_path();
        }

    private:
        bool m_depth_buffer{ false };
        bool m_stencil_buffer{ false };
        bool m_float_buffer{ false };
        std::unique_ptr<nvg::Context> m_nvg_context{ nullptr };
        text::font::Map m_font_map{};
    };
}
