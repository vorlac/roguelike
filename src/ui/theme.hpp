#pragma once

#include "ds/color.hpp"
#include "ds/refcounted.hpp"
#include "graphics/text.hpp"
#include "ui/icons.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    class Theme final : public ds::refcounted
    {
    public:
        std::string_view label_font_name{ text::font::style::Sans };
        std::string_view button_font_name{ text::font::style::Sans };
        std::string_view tooltip_font_name{ text::font::style::SansBold };
        std::string_view dialog_title_font_name{ text::font::style::SansBold };
        std::string_view form_group_font_name{ text::font::style::Mono };

        f32 icon_scale{ 1.0f };
        f32 tab_border_width{ 0.75f };

        f32 tooltip_width{ 150.0f };

        f32 standard_font_size{ 16.0f };
        f32 tooltip_font_size{ 18.0f };
        f32 button_font_size{ 20.0f };
        f32 text_box_font_size{ 20.0f };
        f32 dialog_title_font_size{ 24.0f };
        f32 form_group_font_size{ 24.0f };
        f32 form_widget_font_size{ 18.0f };
        f32 form_pre_group_spacing{ 15.0f };
        f32 form_post_group_spacing{ 15.0f };
        f32 form_variable_spacing{ 10.0f };

        f32 label_font_size{ 38.0f };

        f32 dialog_corner_radius{ 5.0f };
        f32 dialog_header_height{ 40.0f };
        f32 dialog_drop_shadow_size{ 15.0f };

        f32 button_corner_radius{ 2.5f };

        f32 tab_inner_margin{ 5.0f };
        f32 tab_min_button_width{ 20.0f };
        f32 tab_max_button_width{ 160.0f };
        f32 tab_control_width{ 20.0f };
        f32 tab_button_horizontal_padding{ 10.0f };
        f32 tab_button_vertical_padding{ 2.0f };

        ds::color<f32> drop_shadow{ 0, 0, 0, 128 };
        ds::color<f32> text_shadow{ 0, 0, 0, 128 };
        ds::color<f32> dialog_shadow{ 0, 0, 0, 128 };
        ds::color<f32> transparent{ 0, 0, 0, 0 };
        ds::color<f32> border_dark{ 29, 29, 29, 255 };
        ds::color<f32> border_light{ 92, 92, 92, 255 };
        ds::color<f32> border_medium{ 35, 35, 35, 255 };
        ds::color<f32> text_color{ Colors::LightGrey };
        ds::color<f32> disabled_text_color{ Colors::DarkGrey };
        ds::color<f32> text_shadow_color{ Colors::Black };
        ds::color<f32> icon_color{ Colors::LightGrey };
        ds::color<f32> label_font_color{ Colors::White };

        ds::color<f32> button_gradient_top_focused{ 64, 64, 64 };
        ds::color<f32> button_gradient_bot_focused{ 48, 48, 48, 255 };
        ds::color<f32> button_gradient_top_unfocused{ 74, 74, 74, 255 };
        ds::color<f32> button_gradient_bot_unfocused{ 58, 58, 58, 255 };
        ds::color<f32> button_gradient_top_pushed{ 41, 41, 41, 255 };
        ds::color<f32> button_gradient_bot_pushed{ 29, 29, 29, 255 };

        ds::color<f32> dialog_fill_unfocused{ 43, 43, 43, 230 };
        ds::color<f32> dialog_fill_focused{ 45, 45, 45, 230 };
        ds::color<f32> dialog_title_unfocused{ 220, 220, 220, 160 };
        ds::color<f32> dialog_title_focused{ 255, 255, 255, 190 };
        ds::color<f32> dialog_header_gradient_top{ 74, 74, 74, 255 };
        ds::color<f32> dialog_header_gradient_bot{ 58, 58, 58, 255 };
        ds::color<f32> dialog_header_sep_top{ 92, 92, 92, 255 };
        ds::color<f32> dialog_header_sep_bot{ 29, 29, 29, 255 };
        ds::color<f32> dialog_popup_fill{ 50, 50, 50, 255 };
        ds::color<f32> dialog_popup_transparent{ 50, 50, 50, 0 };

        Icon::ID check_box_icon{ Icon::Check };
        Icon::ID message_information_icon{ Icon::InfoCircle };
        Icon::ID message_question_icon{ Icon::QuestionCircle };
        Icon::ID message_warning_icon{ Icon::ExclamationTriangle };
        Icon::ID message_alt_button_icon{ Icon::PlusCircle };
        Icon::ID message_primary_button_icon{ Icon::Check };
        Icon::ID popup_chevron_right_icon{ Icon::ChevronRight };
        Icon::ID popup_chevron_left_icon{ Icon::ChevronLeft };
        Icon::ID text_box_up_icon{ Icon::ChevronUp };
        Icon::ID text_box_down_icon{ Icon::ChevronDown };
    };
}
