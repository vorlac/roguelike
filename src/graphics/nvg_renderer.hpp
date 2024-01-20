#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/assert.hpp"
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
        using FontInfo = std::pair<std::string_view, std::basic_string_view<u8>>;

    public:
        NVGRenderer();

        void flush(ds::dims<i32> viewport, f32 pixel_ratio = 1.0f);
        void render_frame(ds::dims<f32> render_size, f32 pixel_ratio, auto&& render_func);
        void begin_frame(ds::dims<f32> render_size, f32 pixel_ratio = 1.0f);
        void end_frame();

        void push_render_state();
        void pop_render_state();
        void scoped_state_draw(auto&& draw_func);

        ui::font::Handle load_font(std::string_view font_name,
                                   std::basic_string_view<u8>&& font_ttf);

        void load_fonts(std::vector<FontInfo>&& fonts);
        void set_text_properties(std::string_view& font_name, f32 font_size,
                                 ui::Text::Alignment alignment) const;

        ds::dims<f32> get_text_size(const std::string& text) const;
        ds::dims<f32> get_text_size(
            const std::string& text, std::string_view& font_name, f32 font_size,
            ui::Text::Alignment alignment = ui::Text::Alignment::HCenterVMiddle) const;
        ds::rect<f32> get_text_box_rect(
            const std::string& text, const ds::point<f32>& pos, std::string_view& font_name,
            f32 font_size, f32 fold_width,
            ui::Text::Alignment alignment = ui::Text::Alignment::HLeftVTop) const;

        void draw_rect_outline(ds::rect<f32> rect, f32 stroke_width, ds::color<f32> color,
                               ui::Outline type);

        nvg::NVGcontext* nvg_context() const;

    private:
        bool m_depth_buffer{ false };
        bool m_stencil_buffer{ false };
        bool m_float_buffer{ false };
        nvg::NVGcontext* m_nvg_context{ nullptr };
        ui::font::Map m_font_map{};
    };
}
