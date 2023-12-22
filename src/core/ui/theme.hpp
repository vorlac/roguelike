#pragma once

#include <nanovg.h>

#include "core/ui/ui_icons.hpp"
#include "ds/color.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    class theme : public ds::refcounted
    {
    public:
        theme(NVGcontext* nvg_context)
        {
            bool font_load_success = true;
            font_load_success &= m_font_sans_regular != -1;
            font_load_success &= m_font_sans_bold != -1;
            font_load_success &= m_font_icons != -1;
            font_load_success &= m_font_mono_regular != -1;
            runtime_assert(font_load_success, "Failed to load fonts");
        }

        i32 m_font_sans_regular;
        i32 m_font_sans_bold;
        i32 m_font_icons;
        i32 m_font_mono_regular;

        f32 m_icon_scale{ 1.0f };
        f32 m_tab_border_width{ 0.75f };

        i32 m_standard_font_size{ 16 };
        i32 m_button_font_size{ 20 };
        i32 m_text_box_font_size{ 20 };
        i32 m_window_corner_radius{ 2 };
        i32 m_window_header_height{ 30 };
        i32 m_window_drop_shadow_size{ 10 };
        i32 m_button_corner_radius{ 2 };
        i32 m_tab_inner_margin{ 5 };
        i32 m_tab_min_button_width{ 20 };
        i32 m_tab_max_button_width{ 160 };
        i32 m_tab_control_width{ 20 };
        i32 m_tab_button_horizontal_padding{ 10 };
        i32 m_tab_button_vertical_padding{ 2 };

        ds::color<u8> m_drop_shadow{ 0, 0, 0, 128 };
        ds::color<u8> m_transparent{ 0, 0, 0, 0 };
        ds::color<u8> m_border_dark{ 29, 29, 29, 255 };
        ds::color<u8> m_border_light{ 92, 92, 92, 255 };
        ds::color<u8> m_border_medium{ 35, 35, 35, 255 };
        ds::color<u8> m_text_color{ 255, 255, 255, 160 };
        ds::color<u8> m_disabled_text_color{ 255, 255, 255, 80 };
        ds::color<u8> m_text_shadow_color{ 0, 0, 0, 0 };
        ds::color<u8> m_icon_color{ m_text_color };

        ds::color<u8> m_button_gradient_top_focused{ 64, 64, 64, 255 };
        ds::color<u8> m_button_gradient_bot_focused{ 48, 48, 48, 255 };
        ds::color<u8> m_button_gradient_top_unfocused{ 74, 74, 74, 255 };
        ds::color<u8> m_button_gradient_bot_unfocused{ 58, 58, 58, 255 };
        ds::color<u8> m_button_gradient_top_pushed{ 41, 41, 41, 255 };
        ds::color<u8> m_button_gradient_bot_pushed{ 29, 29, 29, 255 };

        ds::color<u8> m_window_fill_unfocused{ 43, 43, 43, 230 };
        ds::color<u8> m_window_fill_focused{ 45, 45, 45, 230 };
        ds::color<u8> m_window_title_unfocused{ 220, 220, 220, 160 };
        ds::color<u8> m_window_title_focused{ 255, 255, 255, 190 };
        ds::color<u8> m_window_header_gradient_top{ m_button_gradient_top_unfocused };
        ds::color<u8> m_window_header_gradient_bot{ m_button_gradient_bot_unfocused };
        ds::color<u8> m_window_header_sep_top{ m_border_light };
        ds::color<u8> m_window_header_sep_bot{ m_border_dark };
        ds::color<u8> m_window_popup{ 50, 50, 50, 255 };
        ds::color<u8> m_window_popup_transparent{ 50, 50, 50, 0 };

        i32 m_check_box_icon{ FA_CHECK };
        i32 m_message_information_icon{ FA_INFO_CIRCLE };
        i32 m_message_question_icon{ FA_QUESTION_CIRCLE };
        i32 m_message_warning_icon{ FA_EXCLAMATION_TRIANGLE };
        i32 m_message_alt_button_icon{ FA_PLUS_CIRCLE };
        i32 m_message_primary_button_icon{ FA_CHECK };
        i32 m_popup_chevron_right_icon{ FA_CHEVRON_RIGHT };
        i32 m_popup_chevron_left_icon{ FA_CHEVRON_LEFT };
        i32 m_text_box_up_icon{ FA_CHEVRON_UP };
        i32 m_text_box_down_icon{ FA_CHEVRON_DOWN };
    };

}
