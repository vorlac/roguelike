#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/ui/theme.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "utils/numeric.hpp"

namespace rl {

    class NVGRenderer
    {
    public:
        using FontInfo = std::pair<std::string, std::basic_string_view<u8>>;

    public:
        NVGRenderer();

        void flush(const ds::dims<f32>& viewport, f32 pixel_ratio = 1.0f) const;
        void begin_frame(const ds::dims<f32>& render_size, f32 pixel_ratio = 1.0f) const;
        void end_frame() const;

        void save_state() const;
        void restore_state() const;

        ui::Font::ID load_font(const std::string_view& font_name,
                               const std::basic_string_view<u8>& font_ttf) const;

        void load_fonts(const std::vector<FontInfo>& fonts);
        void set_text_properties(const std::string_view& font_name, f32 font_size,
                                 ui::Text::Alignment alignment) const;

        [[nodiscard]] ds::dims<f32> get_text_size(const std::string& text) const;
        [[nodiscard]] ds::dims<f32> get_text_size(
            const std::string& text, const std::string_view& font_name, f32 font_size,
            ui::Text::Alignment alignment = ui::Text::Alignment::HCenterVMiddle) const;

        [[nodiscard]] ds::rect<f32> get_text_box_rect(
            const std::string& text, const ds::point<f32>& pos, const std::string_view& font_name,
            f32 font_size, f32 fold_width,
            ui::Text::Alignment alignment = ui::Text::Alignment::HLeftVTop) const;

        void draw_rect_outline(const ds::rect<f32>& rect, f32 stroke_width,
                               const ds::color<f32>& color, ui::Outline type) const;

        [[nodiscard]] nvg::NVGcontext* context() const;

    public:
        template <std::invocable TCallable>
        void scoped_draw(TCallable&& callable)
        {
            this->save_state();
            std::invoke(std::forward<TCallable>(callable));
            this->restore_state();
        }

        template <std::invocable TCallable>
        void draw_frame(TCallable&& callable, const ds::dims<f32>& render_size,
                        const f32 pixel_ratio)
        {
            this->begin_frame(render_size, pixel_ratio);
            std::invoke(std::forward<TCallable>(callable));
            this->end_frame();
        }

    private:
        bool m_depth_buffer{ false };
        bool m_stencil_buffer{ false };
        bool m_float_buffer{ false };
        nvg::NVGcontext* m_nvg_context{ nullptr };
        ui::Font::Map m_font_map{};
    };
}
